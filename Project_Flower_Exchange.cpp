#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <map>

using namespace std;

// string orders_file_path = "order.csv"; // file path
ofstream execution_rep("execution_rep.csv");
unsigned int t_count = 0;

string instruments[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

string set_transaction_time()
{
    auto now = chrono::system_clock::now();
    time_t currentTime = chrono::system_clock::to_time_t(now);

    tm *timeInfo = localtime(&currentTime);
    ostringstream oss;
    oss << put_time(timeInfo, "%Y%m%d-%H%M%S");
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    oss << '.' << setw(3) << setfill('0') << milliseconds.count();

    return oss.str();
}

class Order
{
public:
    int temp_id; // indicator for the order of the order

    string client_id;
    string order_id;
    string instrument;
    int side;
    double price;
    int quantity;
    int status;
    string reason;
    string transaction_time;

    int remaining_quantity;

    Order()
    {
        status = 0;
        transaction_time = set_transaction_time();
    };

    bool is_valid(); // check whether the order is valid or not.
                     // if not valid, set status, reason and write invalid order to execution report

    void write_exec_rep(ofstream &, int); // write executed order to execution report
    void write_exec_rep(ofstream &);      // overloaded write_exec_rep function
};

bool Order::is_valid()
{
    if (client_id.empty())
    {
        status = 1;
        reason = "Invalid client id";
        return false;
    }
    if (find(begin(instruments), end(instruments), instrument) == end(instruments))
    {
        status = 1;
        reason = "Invalid instrument";
        return false;
    }
    if (side != 1 && side != 2)
    {
        status = 1;
        reason = "Invalid side";
        return false;
    }
    if (price <= 0)
    {
        status = 1;
        reason = "Invalid price";
        return false;
    }
    if (quantity % 10 != 0 || quantity > 1000 || quantity <= 0)
    {
        status = 1;
        reason = "Invalid quantity";
        return false;
    }
    return true;
}

void Order::write_exec_rep(ofstream &fout, int executed_quantity)
{
    transaction_time = set_transaction_time();
    string stat = "";
    if (status == 0)
        stat = "New";
    else if (status == 1)
        stat = "Reject";
    else if (status == 2)
        stat = "Fill";
    else if (status == 3)
        stat = "PFill";
    fout << order_id << "," << client_id << "," << instrument << "," << side << "," << price << "," << executed_quantity << "," << stat << "," << reason << "," << transaction_time << endl;
}

void Order::write_exec_rep(ofstream &fout)
{
    write_exec_rep(fout, remaining_quantity);
}

bool compare_orders(const Order &ord1, const Order &ord2)
{
    if (ord1.price == ord2.price)
    {
        return ord1.temp_id < ord2.temp_id; // if prices are equal, order which came first should be executed first
    }
    if (ord1.side == 1)
    {
        return ord1.price < ord2.price;
    }
    else
    {
        return ord1.price > ord2.price;
    }
}

vector<Order> order_book_buy_rose;
vector<Order> order_book_sell_rose;
vector<Order> order_book_buy_lavender;
vector<Order> order_book_sell_lavender;
vector<Order> order_book_buy_lotus;
vector<Order> order_book_sell_lotus;
vector<Order> order_book_buy_tulip;
vector<Order> order_book_sell_tulip;
vector<Order> order_book_buy_orchid;
vector<Order> order_book_sell_orchid;

// five order books for five instruments
vector<Order> *order_books[5][2] = {
    {&order_book_buy_rose, &order_book_sell_rose},
    {&order_book_buy_lavender, &order_book_sell_lavender},
    {&order_book_buy_lotus, &order_book_sell_lotus},
    {&order_book_buy_tulip, &order_book_sell_tulip},
    {&order_book_buy_orchid, &order_book_sell_orchid},
};

int main(int argc, char const *argv[])
{
    // make_heap(order_book_buy_rose.begin(), order_book_buy_rose.end(), compare_orders);
    // make_heap(order_book_sell_rose.begin(), order_book_sell_rose.end(), compare_orders);
    // make_heap(order_book_buy_lavender.begin(), order_book_buy_lavender.end(), compare_orders);
    // make_heap(order_book_sell_lavender.begin(), order_book_sell_lavender.end(), compare_orders);
    // make_heap(order_book_buy_lotus.begin(), order_book_buy_lotus.end(), compare_orders);
    // make_heap(order_book_sell_lotus.begin(), order_book_sell_lotus.end(), compare_orders);
    // make_heap(order_book_buy_tulip.begin(), order_book_buy_tulip.end(), compare_orders);
    // make_heap(order_book_sell_tulip.begin(), order_book_sell_tulip.end(), compare_orders);
    // make_heap(order_book_buy_orchid.begin(), order_book_buy_orchid.end(), compare_orders);
    // make_heap(order_book_sell_orchid.begin(), order_book_sell_orchid.end(), compare_orders);

    string file_path;
    cout << "Enter file path : ";
    cin >> file_path;

    cout << "started..." << endl;
    auto start = chrono::high_resolution_clock::now();

    ifstream orders(file_path);
    // ifstream orders(orders_file_path);
    if (!orders.is_open())
    {
        // cout << "Invalid file path : " << orders_file_path << endl;
        cout << "Invalid file path : " << file_path << endl;
        return 1;
    }
    string line;
    getline(orders, line); // skip the first line

    execution_rep << "order_id,client_order_id,instrument,side,price,quantity,exec_status,reason,transaction_time" << endl;

    unsigned int count = 1;
    while (getline(orders, line))
    {
        Order order;
        stringstream line_string(line);
        static string temp;

        order.order_id = "ord" + to_string(count++);
        t_count = count;
        order.temp_id = t_count;

        getline(line_string, order.client_id, ',');
        getline(line_string, order.instrument, ',');

        temp = "";
        getline(line_string, temp, ',');
        order.side = stoi(temp.c_str());

        temp = "";
        getline(line_string, temp, ',');
        order.quantity = stoi(temp.c_str());

        temp = "";
        getline(line_string, temp, ',');
        order.price = stod(temp.c_str());

        order.remaining_quantity = order.quantity;

        if (!order.is_valid())
        {
            order.write_exec_rep(execution_rep);
            continue;
        }
        else
        {
            int index = (int)(find(begin(instruments), end(instruments), order.instrument) - begin(instruments));
            vector<Order> **orderBook = order_books[index];
            // vector<Order> &buy_side = *orderBook[0];
            // vector<Order> &sell_side = *orderBook[1];

            if (order.side == 1)
            {
                // buy order
                vector<Order> &sell_side = *orderBook[1];
                while (!sell_side.empty() && sell_side.front().price <= order.price) // if there are sell orders in the order book and the price of the sell order is less than or equal to the price of the buy order
                {
                    Order &s_order = sell_side.front();
                    if (order.remaining_quantity == s_order.remaining_quantity) // quantity of the buy order is equal to the quantity of the sell order
                    {
                        order.status = 2;
                        s_order.status = 2;
                        order.price = s_order.price;
                        order.write_exec_rep(execution_rep);
                        s_order.write_exec_rep(execution_rep);
                        order.remaining_quantity = 0;
                        s_order.remaining_quantity = 0;
                        pop_heap(sell_side.begin(), sell_side.end(), compare_orders);
                        sell_side.pop_back();
                        break;
                    }
                    else if (order.remaining_quantity > s_order.remaining_quantity) // quantity of the buy order is greater than the quantity of the sell order
                    {
                        double temp = order.price;
                        order.status = 3;
                        s_order.status = 2;
                        order.price = s_order.price;
                        order.write_exec_rep(execution_rep, s_order.remaining_quantity);
                        s_order.write_exec_rep(execution_rep);
                        order.remaining_quantity = order.remaining_quantity - s_order.remaining_quantity;
                        s_order.remaining_quantity = 0;
                        order.price = temp;
                        pop_heap(sell_side.begin(), sell_side.end(), compare_orders);
                        sell_side.pop_back();
                    }
                    else // quantity of the buy order is less than the quantity of the sell order
                    {
                        order.status = 2;
                        s_order.status = 3;
                        order.price = s_order.price;
                        order.write_exec_rep(execution_rep);
                        s_order.write_exec_rep(execution_rep, order.remaining_quantity);
                        s_order.remaining_quantity = s_order.remaining_quantity - order.remaining_quantity;
                        pop_heap(sell_side.begin(), sell_side.end(), compare_orders);
                        sell_side.pop_back();
                        s_order.temp_id = t_count++;
                        sell_side.push_back(s_order);
                        push_heap(sell_side.begin(), sell_side.end(), compare_orders);
                        order.remaining_quantity = 0;
                        break;
                    }
                }
                //
                if (order.status == 0) // if the order is not executed (new order)
                {
                    order.write_exec_rep(execution_rep);
                }
                //
                if (order.remaining_quantity > 0.0) // if the order is not executed completely
                {
                    vector<Order> &buy_side = *orderBook[0];
                    order.temp_id = t_count++;
                    buy_side.push_back(order);
                    push_heap(buy_side.begin(), buy_side.end(), compare_orders);
                }
            }
            else if (order.side == 2) // sell order
            {
                vector<Order> &buy_side = *orderBook[0];
                while (!buy_side.empty() && buy_side.front().price >= order.price) // if there are buy orders in the order book and the price of the buy order is greater than or equal to the price of the sell order
                {
                    Order &b_order = buy_side.front();
                    if (order.remaining_quantity == b_order.remaining_quantity) // quantity of the sell order is equal to the quantity of the buy order
                    {
                        order.status = 2;
                        b_order.status = 2;
                        order.price = b_order.price;
                        order.write_exec_rep(execution_rep);
                        b_order.write_exec_rep(execution_rep);
                        order.remaining_quantity = 0;
                        b_order.remaining_quantity = 0;
                        pop_heap(buy_side.begin(), buy_side.end(), compare_orders);
                        buy_side.pop_back();
                        break;
                    }
                    else if (order.remaining_quantity > b_order.remaining_quantity) // quantity of the sell order is greater than the quantity of the buy order
                    {
                        double temp = order.price;
                        order.status = 3;
                        b_order.status = 2;
                        order.price = b_order.price;
                        order.write_exec_rep(execution_rep, b_order.remaining_quantity);
                        order.remaining_quantity = order.remaining_quantity - b_order.remaining_quantity;
                        b_order.write_exec_rep(execution_rep);
                        b_order.remaining_quantity = 0;
                        order.price = temp;
                        pop_heap(buy_side.begin(), buy_side.end(), compare_orders);
                        buy_side.pop_back();
                    }
                    else // quantity of the sell order is less than the quantity of the buy order
                    {
                        order.status = 2;
                        b_order.status = 3;
                        order.price = b_order.price;
                        order.write_exec_rep(execution_rep);
                        b_order.write_exec_rep(execution_rep, order.remaining_quantity);
                        b_order.remaining_quantity = b_order.remaining_quantity - order.remaining_quantity;
                        pop_heap(buy_side.begin(), buy_side.end(), compare_orders);
                        buy_side.pop_back();
                        b_order.temp_id = t_count++;
                        buy_side.push_back(b_order);
                        push_heap(buy_side.begin(), buy_side.end(), compare_orders);
                        order.remaining_quantity = 0;
                        break;
                    }
                }

                if (order.status == 0) // if the order is not executed (new order)
                {
                    order.write_exec_rep(execution_rep);
                }

                if (order.remaining_quantity > 0.0) // if the order is not executed completely
                {
                    vector<Order> &sell_side = *orderBook[1];
                    order.temp_id = t_count++;
                    sell_side.push_back(order);
                    push_heap(sell_side.begin(), sell_side.end(), compare_orders);
                }
            }
        }
    }
    orders.close();
    execution_rep.close();

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Execution time: " << duration.count() / 1000.0 << "s" << endl;

    return 0;
}
