//This program is designed to function as a food ordering application for Pho of Fort Wayne resturant.

#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include <cstdlib>

using namespace std;
void Appetizers();

struct menuItemAppetizers{
    // this constructor allows us to add items to the vector for the Appetizers
    //part of the menu options
    menuItemAppetizers(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};

int main()
{
    int choice;

    cout << endl;
        //Welcome page for the Pho of Fort Wayne
    cout << "                      Welcome to Pho of Fort Wayne!       " << endl;
    this_thread::sleep_for(2s);
    cout << endl;
    cout <<"    Serving Authenic Vietnamese food in Fort Wayne, Indiana since 2009   " << endl;
    this_thread::sleep_for(2s);
    cout << "               Press the enter key to continue     ";
    cin.ignore();
    //system("CLS")
    //cout << string(50, '\n');

    //Listing out the main menu choices!

    cout << "What would you like to order? :" << endl;
    cout << "   For Appetizers enter 1" << endl;
    //cout << endl;
    cout << "   For Bun - Rice Vermicell enter 2" << endl;
    //cout << endl;
    cout << "   For Pho - Rice Nooble Soup enter 3 " << endl;
    cout << "   For Mi - Egg Noodles enter 4 " << endl;
    cout << "   For Com Dia - Gourmet Rice Dishes enter 5 " << endl;
    cout << "   For Banhi Mi  - Vietnamese Sandwiches enter 6 " << endl;
    cout << "   For Drinks enter 7 " << endl;
    cout << "Enter choice between 1 - 7:";
    //cin.ignore();
    cin >> choice;
    
    //Making choice the choice stays between 1- 7
    while (!((choice > 0) && (choice < 8)))
    {
        //cin.ignore();
        cout << endl;
        cout << "Outside order menu range!" << endl;
        cout << "Here is the Main menu again :" << endl;
        cout << endl;
        cout << "What would you like to order? :" << endl;
        cout << "   For Appetizers enter 1" << endl;
    //cout << endl;
        cout << "   For Bun - Rice Vermicell enter 2" << endl;
    //cout << endl;
        cout << "   For Pho - Rice Nooble Soup enter 3 " << endl;
        cout << "   For Mi - Egg Noodles enter 4 " << endl;
        cout << "   For Com Dia - Gourmet Rice Dishes enter 5 " << endl;
        cout << "   For Banhi Mi  - Vietnamese Sandwiches enter 6 " << endl;
        cout << "   For Drinks enter 7 " << endl;
        cout << "Enter choice between 1 - 7: ";
        cin >> choice;
        
    }
    //return 0;
   if (choice == 1)
    {
        Appetizers();

    }
    /*else if (choice == 2)
    {

    } */
}


void Appetizers()
{
    int num1, order;
    //Adding the Appetizers items to the menu!
    vector<menuItemAppetizers> appetizers;
    appetizers.push_back(menuItemAppetizers("Vietnamese Egg Rolls(2) - Groud pork, shrimp, carrots, taro, served with lettuce and a sweet, spicy sauce. ", 2.99));
    appetizers.push_back(menuItemAppetizers("Vietnamese Spring Rolls(2) - Pork, shrimp, rice, noodles and mint leaves wrapped in rice paper with a peanut sauce. ", 4.99));
    appetizers.push_back(menuItemAppetizers("Nem Nuong Cuon(2) - Grilled pork sausage spring roll. ", 4.99));
    appetizers.push_back(menuItemAppetizers("Bo Nuong - Short rib beef marinated and grilled. Served with lettuce and cilantro. ", 7.99));
    appetizers.push_back(menuItemAppetizers("Myo Chien - Lightly coated deep fried calamari, stir-tried with fresh garllic and chill pepper. Served with lettuce and cilantro. ", 8.99));
    appetizers.push_back(menuItemAppetizers("Hen Nuog - Marinated mussels grilled with green onions. ", 9.99));
    appetizers.push_back(menuItemAppetizers("Chim Cut Quay - quails slowly roasted to perfection. ",9.99));
    appetizers.push_back(menuItemAppetizers("Canh Ga Nudo Mam(6) - Stir fried chicken with fish sauce. ", 6.99));
    appetizers.push_back(menuItemAppetizers("Seasame Chicken Wings(6) - Deep fried chicken wings cooked in sesame sauce. ", 6.99));
    appetizers.push_back(menuItemAppetizers("House Chicken Wings(6) - Deep fried, spicy chicken wings stir-fried in our special garlic seasoning. ", 6.99));


    //Displaying the Appetizer Menu
    cout << "Appetizers" << endl;
    cout.setf(ios::left);
    for (unsigned int x =0; x < appetizers.size(); x++)
        cout << x+1 << "." << setw(20) << appetizers[x].itemName << "$" << appetizers[x].itemCost << endl;
        //cout << endl;
    cout << endl;
    cout << "What would you like to order from the Appetizer menu [enter between 1 - 10]?" << endl;

    cin >> order;
    while (!((order > 0) && (order < 11) ))
    {
        cout << "Invalid order number! Enter a valid order number between 1-10" << endl;
        cin >> order;
    }

    cout << "How many of " << appetizers[order - 1] << " would you like?" << endl;




    //clrscr();


} 