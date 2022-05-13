//
//  main.cpp
//  M05ProgrammingAssignment2
//
//  Created by Mwai Banda on 5/2/22.
//

#include <iostream>
#include <string>
#include <iomanip>
#include "sqlite3.h"
#include <list>
#include <set>
#include <map>

using namespace std;

struct Customer {
    int cus_num;
    string cus_fname, cus_lname;
    string cus_phone, cus_initial;
    int cus_areacode;
    double cus_balance;
};

struct Product {
    string p_code, p_description;
    string p_in_date;
    int p_qoh, p_min;
    double p_price, p_discount;
    int v_code;
};


void printMainMenu();
void viewInvoice(sqlite3 *);
void addInvoice(sqlite3 *);
void viewCustomer(sqlite3 *);
Customer* getCustomers(sqlite3 * db);
Product* getProducts(sqlite3 * db);
int mainMenu();


const int cinTerminator = -1;
const int CUSTOMERSMAX = 10;
const int PRODUCTSMAX = 15;
const string TOTALKEY = "NetTotal";


int main()
{
    int choice;
    
    sqlite3 *mydb;
    
    int rc;
    
    rc = sqlite3_open("/Users/mwaibanda/Repository/Main Projects/Command Line Projects/M05ProgrammingAssignment2/M05ProgrammingAssignment2/SaleCo.db", &mydb);
    
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(mydb));
        return(1);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    
    cout << "Welcome to SaleCo" << endl;
    
    choice = mainMenu();
    while (true)
    {
        switch (choice)
        {
            case 1: viewInvoice(mydb); break;
            case 2: addInvoice(mydb); break;
            case 3: viewCustomer(mydb); break;
            case -1: return 0;
            default: cout << "That is not a valid choice." << endl;
        }
        cout << "\n\n";
        choice = mainMenu();
    }
    
}

void printMainMenu()
{
    cout << "Please choose an option (enter -1 to quit):  " << endl;
    cout << "1. View an invoice" << endl;
    cout << "2. Add an invoice" << endl;
    cout << "3. View Customer Information" << endl;
    cout << "Enter Choice: ";
}

int mainMenu()
{
    int choice = 0;
    
    printMainMenu();
    cin >> choice;
    while ((!cin || choice < 1 || choice > 3) && choice  != -1)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(1000,'/n');
        }
        cout << "That is not a valid choice." << endl << endl;
        printMainMenu();
        cin >> choice;
    }
    return choice;
}

void viewInvoice(sqlite3 * db)
{
    string query = "SELECT INVOICE.INV_NUMBER, INVOICE.INV_DATE, CUSTOMER.CUS_FNAME, CUSTOMER.CUS_LNAME ";
    query += "FROM INVOICE JOIN CUSTOMER ON INVOICE.CUS_CODE = CUSTOMER.CUS_CODE;";
    sqlite3_stmt *pRes;
    string m_strLastError;
    string query2;
    string inv_number;
    string inv_date;
    string cus_fname,cus_lname;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        m_strLastError = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << m_strLastError << endl;
        return;
    }
    else
    {
        cout << "Please choose the invoice you want to see:" << endl;
        int columnCount = sqlite3_column_count(pRes);
        int i = 1, choice;
        sqlite3_stmt *pRes2;
        cout << left;
        while (sqlite3_step(pRes) == SQLITE_ROW)
        {
            cout << i << ". " << sqlite3_column_text(pRes, 0);
            cout << endl;
            i++;
        }
        do
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(1000, '\n');
            }
            cin >> choice;
            if (!cin || choice < 1 || choice > i)
                cout << "That is not a valid choice! Try Again!" << endl;
        } while (!cin);
        
        sqlite3_reset(pRes);
        for (int i = 0; i < choice; i++)
            sqlite3_step(pRes);
        inv_number = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 0));
        inv_date = reinterpret_cast<const char*>(sqlite3_column_text(pRes,1));
        cus_fname = reinterpret_cast<const char*>(sqlite3_column_text(pRes,2));
        cus_lname = reinterpret_cast<const char*>(sqlite3_column_text(pRes,3));
        sqlite3_finalize(pRes);
        //need to provide the query to select the customers with the chosen region from the database
        query2 = "SELECT PRODUCT.P_DESCRIPT as Product ,LINE.LINE_PRICE as Price, LINE.LINE_UNITS as Units ";
        query2 += "FROM LINE ";
        query2 += "JOIN PRODUCT on line.P_CODE = PRODUCT.P_CODE  ";
        query2 += "WHERE LINE.INV_NUMBER = '" + inv_number + "';";
        
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
        {
            m_strLastError = sqlite3_errmsg(db);
            sqlite3_finalize(pRes2);
            cout << "There was an error: " << m_strLastError << endl;
            return;
        }
        else
        {
            cout << "Invoice #: " << inv_number << endl;
            cout << "Invoice Date: " << inv_date << endl;
            cout << "Customer: " << cus_fname << " " << cus_lname << endl;
            columnCount = sqlite3_column_count(pRes2);
            cout << left;
            for (int i = 0; i < columnCount; i++)
            {
                cout << "|" << setw(25) << sqlite3_column_name(pRes2, i);
            }
            cout << "|" << endl;
            
            while (sqlite3_step(pRes2) == SQLITE_ROW)
            {
                for (int i = 0; i < columnCount; i++)
                {
                    if (sqlite3_column_type(pRes2, i) != SQLITE_NULL)
                        cout << "|" << setw(25) << sqlite3_column_text(pRes2, i);
                    else
                        cout << "|" << setw(25) << " ";
                }
                cout << "|" << endl;
            }
            sqlite3_finalize(pRes2);
        }
        
    }
}

void addInvoice(sqlite3 * db)
{
    
    /* Take the program and add a menu option for adding an invoice.  This should call a function that sets up to run a transaction that will insert everything needed for the invoice.  You should start by asking which customer the invoice is for by using a printed menu.  Next you will need to find out which products will be on the invoice and the quantities (these make up the line records in the database). You should again use printed menus to gather this information.  Once the user has entered all the information You will need to insert the invoice record and the line records, update the customer balance, and update the product quantity on hand using a transaction sent to the database.*/
    string query = "SELECT * FROM CUSTOMER", errorMsg;
  

    sqlite3_stmt *pRes;
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << errorMsg << endl;
        return;
    }
    else
    {
        cout << "\nPlease choose the customer you want to create an invoice for:" << endl;
        int i = 1, choice;
        cout << left;
        Customer* customers;
        customers = getCustomers(db);
        do
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(1000, '\n');
            }
            printf("Enter choice: ");
            cin >> choice;
            if (!cin || choice < 1 || choice > i)
                cout << "That is not a valid choice! Try Again!" << endl;
        } while (!cin);
        choice--;
        Customer selectedCustomer = customers[choice];
        delete []customers;
        
        printf("\nWelcome %s, Select Products:\n", selectedCustomer.cus_fname.c_str());
        Product* products = getProducts(db);
        int num;
        set<int> productNumbers;
        map<string, int> quantities;
        map<string, double> totals;

        map<string, double>::iterator itr;

        cout << "\nEnter a list of product numbers terminated -1(i.e 1 2 3 -1):\n";
        
        while ((cin >> num) && num != cinTerminator) {
            productNumbers.insert(num);
        }
        
        for(int i : productNumbers) {
            printf("\nEnter quantity for %i. %s, $%g: ", i, products[i - 1].p_description.c_str(), products[i - 1].p_price);
            cin >> quantities[products[i - 1].p_code];
            totals[products[i - 1].p_code] = products[i - 1].p_price * quantities[products[i - 1].p_code];
        }
        printf("\n");
        totals[TOTALKEY] = 0.0;
        for(int i : productNumbers) {
            printf("for [%d] of %d. %s priced at $%g will cost $%g\n", quantities[products[i - 1].p_code], i, products[i - 1].p_description.c_str(), products[i - 1].p_price, totals[products[i - 1].p_code]);
            totals[TOTALKEY] = totals[TOTALKEY] + totals[products[i - 1].p_code];
            
        }
        printf("Your total is %g\n", totals[TOTALKEY]);
        for (itr = totals.begin(); itr != totals.end(); itr++) {
            printf("[%s: %g]", itr->first.c_str(), itr->second);
        }
        cout << endl;
        string transaction = "BEGIN TRANSACTION;"\
        "   INSERT INTO INVOICE VALUES(1009," + to_string(selectedCustomer.cus_num)  + ",12-Jan-22)"\
        
        "   UPDATE CUSTOMER"\
        "   SET CUS_BALANCE=" + to_string(selectedCustomer.cus_balance + totals[selectedCustomer.cus_phone]) + ""\
        "   WHERE CUSTOMER.CUS_NUM=" + to_string(selectedCustomer.cus_num) + ""\
        
        "COMMIT TRANSACTION;";
        
        if (sqlite3_prepare_v2(db, transaction.c_str(), -1, &pRes, NULL) != SQLITE_OK)
        {
            errorMsg = sqlite3_errmsg(db);
            sqlite3_finalize(pRes);
            cout << "There was an error: " << errorMsg << endl;
            return;
        }
        else
        {
        sqlite3_finalize(pRes);
        }
    }
}

void viewCustomer(sqlite3 * db)
{
    /* This function should run a query that gets the customer ID and first and last names for all customers in the database and uses that to display a menu of options to the user.  Based on the user choice the function should print the customer's name, phone number with area code, and balance. */
    string query = "SELECT * FROM CUSTOMER", errorMsg;
    sqlite3_stmt *pRes;
    int cus_num;
    string cus_phone;
    string cus_fname, cus_lname;
    int cus_area;
    double cus_balance;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << errorMsg << endl;
        return;
    }
    else
    {
        cout << "\nPlease choose the customer you want to see:" << endl;
        int i = 1, choice;
        cout << left;
        Customer* customers;
        customers = getCustomers(db);
        do
        {
            if (!cin)
            {
                cin.clear();
                cin.ignore(1000, '\n');
            }
            printf("Enter choice: ");
            cin >> choice;
            if (!cin || choice < 1 || choice > i)
                cout << "That is not a valid choice! Try Again!" << endl;
        } while (!cin);
        choice--;
        
        cus_num = customers[choice].cus_num;
        cus_lname = customers[choice].cus_lname;
        cus_fname = customers[choice].cus_fname;
        cus_area = customers[choice].cus_areacode;
        cus_phone = customers[choice].cus_phone;
        cus_balance = customers[choice].cus_balance;
        
        sqlite3_finalize(pRes);
        
        printf("\n\nID: %i\n", cus_num);
        printf("Name: %s %s\n", cus_fname.c_str(), cus_lname.c_str());
        printf("Phone: %s\n", cus_phone.c_str());
        printf("Area Code: %i\n", cus_area);
        printf("Balance: %f\n", cus_balance);
        delete []customers;
    }
}

Customer* getCustomers(sqlite3 * db){
    Customer * customers = new Customer[CUSTOMERSMAX];
    string query = "SELECT * FROM CUSTOMER", errorMsg;
    sqlite3_stmt *pRes;
    int count = 0;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << errorMsg << endl;
    }
    else
    {
        while (sqlite3_step(pRes) == SQLITE_ROW)
        {
            cout  << count + 1 << ". " << sqlite3_column_text(pRes, 1) << " " << sqlite3_column_text(pRes, 2) << " (" << sqlite3_column_text(pRes, 0) <<  ")";
            cout << endl;
            Customer customer = Customer();
            customer.cus_num = sqlite3_column_int(pRes, 0);
            customer.cus_lname = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 1));
            customer.cus_fname = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 2));
            customer.cus_areacode = sqlite3_column_int(pRes, 4);
            customer.cus_phone = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 5));
            customer.cus_balance = sqlite3_column_double(pRes, 4);
            customers[count] = customer;
            count++;
        }
        
        
        sqlite3_reset(pRes);
    }
    return customers;
}


Product* getProducts(sqlite3 * db){
    Product * products = new Product[PRODUCTSMAX];
    string query = "SELECT * FROM PRODUCT", errorMsg;
    sqlite3_stmt *pRes;
    int count = 0;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_finalize(pRes);
        cout << "There was an error: " << errorMsg << endl;
    }
    else
    {
        while (sqlite3_step(pRes) == SQLITE_ROW)
        {
            cout  << count + 1 << ". " << sqlite3_column_text(pRes, 1) << " $" << sqlite3_column_text(pRes, 5) << " (" << sqlite3_column_text(pRes, 0) <<  ")";
            cout << endl;
            Product product = Product();
            product.p_code = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 0));
            product.p_description = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 1));
            product.p_in_date = reinterpret_cast<const char*>(sqlite3_column_text(pRes, 2));
            product.p_qoh = sqlite3_column_int(pRes, 3);
            product.p_min = sqlite3_column_int(pRes, 4);
            product.p_price = sqlite3_column_double(pRes, 5);
            product.p_discount = sqlite3_column_double(pRes, 6);
            product.v_code = sqlite3_column_int(pRes, 7);
            products[count] = product;
            count++;
        }
        
        
        sqlite3_reset(pRes);
    }
    return products;
}

