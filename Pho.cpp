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
#include <cctype>

using namespace std;
//check out can be zero
struct menuItemAppetizers{
    // this constructor allows us to add items to the vector for the Appetizers
    //part of the menu options
    menuItemAppetizers(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};
struct menuItemRiceNoodleSoup{
    //this constructor allows us to add items to the vector for the Pho - Rice Noodle Soup
    //part of the menu options

    menuItemRiceNoodleSoup(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};

struct menuItemEggNoodles{
    //this constructor allows us to add items to the vector for the Egg Nooble
    //part of te menu options;
    menuItemEggNoodles(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};
struct menuItemBanhMi{
    //this constructor allows us to add items to the vector for the sandwich sub menu
    //part of the menu options

    menuItemBanhMi(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};

struct menuItemDrinks{
    //this constructor allows us to add items to the vector for Drinks menu
    // a sub menu section
    menuItemDrinks(string a, double b): itemName(a), itemCost(b) {}
    string itemName;
    double itemCost;
};

//A flexible approach that would keep track of all the indendepent orders that way and then would generate a final report
struct singleOrder{
    singleOrder(unsigned int io, unsigned int ni): itemOrdered(io), numberItems(ni) {}
    unsigned int itemOrdered;
    unsigned int numberItems;

};

int mainMenu();
int finalPayment(double summingAmount);

int finalPayment( double summingAmount) {

//processing payment and Tips
    const int tax = 0.06;

    int two;
    double newTotal, newTotal1, newTotal2, total;
    total     = summingAmount + (0.06 * summingAmount);
    newTotal  = summingAmount + (0.18 *summingAmount * 0.06);
    newTotal1 = summingAmount + (0.20 * summingAmount * 0.06);
    newTotal2 = summingAmount + (0.24 * summingAmount * 0.06);

    cout << setprecision (2) << fixed << "Your total is " << total << endl;
                
    cout << "Would you like to pick 18% " << "[" << newTotal << "], " << "20% " << "[" << newTotal1 << "], " << "or ";
    cout <<"24% " << "[" << newTotal2 << "] ?" << "(enter 1-3 for the choices) : ";
    cin >> two;

    if (two == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            cout << ".";
            this_thread::sleep_for(1s);
        }
        cout << "Thank you! Your payment of " << newTotal <<" was accpted" << endl;  
    }
    else if (two == 2)
    {
        for (int i = 0; i < 4; i++)
        {
            cout << ".";
            this_thread::sleep_for(1s);
        }
        cout << "Thank you! Your payment of " << newTotal1 <<" was accepted" << endl;  
    }
    else if (two == 3)
    {
        for (int i = 0; i < 4; i++)
        {
            cout << ".";
            this_thread::sleep_for(1s);
        }
        cout << "Thank you! Your payment of " << newTotal2 <<" was accepted" << endl;  
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            cout << ".";
            this_thread::sleep_for(1s);
        }
        cout << "Thank you! Your payment of " << total <<" was accepted" << endl;  
    }         
    return 0;

}

int mainMenu() {
    int choice;
    //Listing out the main menu choices!
    //cout << setw(20) << left;

    cout <<"Welcome to the main menu! ";
    cout << "What would you like to order? :" << endl;
    cout << endl;
    cout << "   For Appetizers [enter 1]" << endl;
    cout << "   For Pho - Rice Nooble Soup [enter 2]" << endl;
    cout << "   For Mi - Egg Noodles [enter 3] " << endl;
    cout << "   For Banhi Mi  - Vietnamese Sandwiches [enter 4] " << endl;
    cout << "   For Drinks [enter 5] " << endl;
    cout << "   For checkout [enter 6] " << endl;
    cout << endl;
    cout << "Enter choice between 1 - 6: ";
    cin >> choice;

    return choice;
}

int main()
{
    int choice;
    int answer;
    int pick;
    int num1, order;

    cout << endl;
        //Welcome page for the Pho of Fort Wayne
    cout << "                      Welcome to Pho of Fort Wayne!       " << endl;
    this_thread::sleep_for(2s);
    cout << endl;
    cout <<"    Serving Authenic Vietnamese food in Fort Wayne, Indiana since 2009   " << endl;
    this_thread::sleep_for(2s);
    cout << "               Press the enter key to continue     ";
    cin.ignore();

    //Adding the Appetizers items to the menu!
    vector<menuItemAppetizers> appetizersMenu;
    appetizersMenu.push_back(menuItemAppetizers("Vietnamese Egg Rolls(2)    - Groud pork, shrimp, carrots, taro, served with lettuce and a sweet, spicy sauce  ", 2.99));
    appetizersMenu.push_back(menuItemAppetizers("Vietnamese Spring Rolls(2) - Pork, shrimp, rice, noodles and mint leaves wrapped in rice paper with a peanut sauce  ", 4.99));
    appetizersMenu.push_back(menuItemAppetizers("Nem Nuong Cuon(2) - Grilled pork sausage spring roll  ", 4.99));
    appetizersMenu.push_back(menuItemAppetizers("Bo Nuong          - Short rib beef marinated and grilled. Served with lettuce and cilantro  ", 7.99));
    appetizersMenu.push_back(menuItemAppetizers("Myo Chien - Lightly coated deep fried calamari, stir-tried with fresh garllic and chill pepper. Served with lettuce and cilantro  ", 8.99));
    appetizersMenu.push_back(menuItemAppetizers("Hen Nuog  - Marinated mussels grilled with green onions  ", 9.99));
    appetizersMenu.push_back(menuItemAppetizers("Chim Cut Quay       - quails slowly roasted to perfection  ", 9.99));
    appetizersMenu.push_back(menuItemAppetizers("Canh Ga Nudo Mam(6) - Stir fried chicken with fish sauce  ", 6.99));
    appetizersMenu.push_back(menuItemAppetizers("Seasame Chicken Wings(6) - Deep fried chicken wings cooked in sesame sauce  ", 6.99));
    appetizersMenu.push_back(menuItemAppetizers("House Chicken Wings(6)   - Deep fried, spicy chicken wings stir-fried in our special garlic seasoning  ", 6.99));

    //Adding the Rice Noodle soup menu!
    vector<menuItemRiceNoodleSoup> phoMenu;
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Dao Biet - Rice noodle soup served with sliced flank steak beef brisket tendom, and meat balls  ", 9.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Tai      - Rice noodle served with sliced flank steak  ", 8.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Ga       - Rice noodle soup served with seasoned chicken breast  ", 8.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Hu Tieu Dao Biet - A seafood combination of shrimp, crab meat, scallops and fish ball ", 9.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Hu Tieu Ga       - Rice noodle with sliced chicken breast ", 8.99));

    //Adding the Egg Noodles to the menu!
    vector<menuItemEggNoodles> eggNoodles;
    eggNoodles.push_back(menuItemEggNoodles("Mi Dao Biet - Egg noodle soup with shrimp, crab meat and pork ", 9.99));
    eggNoodles.push_back(menuItemEggNoodles("Mi Hai San  - Egg noodle soup with shrimp, crab meat, scallops and fish ball ", 10.99));
    eggNoodles.push_back(menuItemEggNoodles("Mi Ga - Egg noodle soup served with sliced chicken breast ", 8.99));
    eggNoodles.push_back(menuItemEggNoodles("Mi Kho Dao Biet - Dried egg noodles with shrimp, crab meat, pork and meant ball served with broth on the side ", 9.99));
    eggNoodles.push_back(menuItemEggNoodles("Mi Xao - Stir Fried egg noodles with seafood and vegatbles ", 14.99));

    //Adding vietnamese sandwiches
    vector<menuItemBanhMi> banhMi;
    banhMi.push_back(menuItemBanhMi("Banh mi Deo Beit - BBQ pork, Vietnames ham and head cheese ", 5.00));
    banhMi.push_back(menuItemBanhMi("Banh Mi Ga - Vietnamese chicken sandwich with pickled vegatables ", 5.00));
    banhMi.push_back(menuItemBanhMi("Banh Mi Bo - Vietnamese beef sandwich with pickled vegatables ", 5.00));
    banhMi.push_back(menuItemBanhMi("Banh Mi Bi - Vietnamese shredded pork sandwich with pickled vegetables ", 5.00));

    //Adding the drink Menu!
    vector<menuItemDrinks> drinksMenu;
    drinksMenu.push_back(menuItemDrinks("Soy Bean Milk  ", 3.00));
    drinksMenu.push_back(menuItemDrinks("Coconut Juice   ", 3.00));
    drinksMenu.push_back(menuItemDrinks("Fresh Lemon Juice & ice ", 4.50));
    drinksMenu.push_back(menuItemDrinks("Cold Vietnamese Coffee ", 4.50));
    drinksMenu.push_back(menuItemDrinks("Lime Juice Lipton ", 4.99));
    drinksMenu.push_back(menuItemDrinks("Bubble Tea - Milk tea, Thai Tea, Taro, Coconut, or Mocha ", 4.99));
    drinksMenu.push_back(menuItemDrinks("Smoothies  - Strawberry, Mango, Lychee, Mocha, Pina Colada ", 4.50));
    drinksMenu.push_back(menuItemDrinks("Coke, Diet Coke, Sprite or Rootbeer ", 1.99));
    drinksMenu.push_back(menuItemDrinks("Beer - Heineken, Corona, Budwiesar, Bud Light or Miller Lite ", 3.10));
    



    vector<singleOrder> orderHistory;
    vector<singleOrder> orderHistory1;
    vector<singleOrder> orderHistory2;
    vector<singleOrder> orderHistory3;
    vector<singleOrder> orderHistory4;
    vector<singleOrder> orderHistory5;
    

            //cin.ignore();
    
    
    //do
    while ( ! ( (pick < 1) && (pick > 6)))
    {
        pick = mainMenu();

        //Displaying the Appetizer Menu
        if (pick == 1)
        {
            //int answer;
            do
            {
                //Displaying the Appetizer Menu
                cout << "       Appetizers Menu" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < appetizersMenu.size(); x++)
                {
                    cout << x+1 << ". " << setw(130) << left << appetizersMenu[x].itemName << "$" ;
                    cout << appetizersMenu[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Appetizer menu [enter between 1 - 10 and enter zero to exit this sub menu]?";
                cin >> answer;
                //cin >> order

                //while (!( (order > 0)  && (order < 11) ) )
                while (!( (answer > -1) && (answer < 11)))
                {
                    cout << "Invalid order number! Enter a valid order number between 0-10: ";
                    //cin >> order;
                    cin >> answer;
                }

                if (answer > 0)
                {
                    cout << "How many of " << appetizersMenu[answer - 1].itemName << "would you like?: ";
                    cin >> num1;

                    //vector<singleOrder> orderHistory;
                    orderHistory.push_back(singleOrder((answer-1), num1));
                     cout << "Would you like to order something else from appetizer menu?[Enter 1 for YES and 0 for No]: ";
                    cin >>answer;
                }
               
            }
            while (answer != 0);

        }
        if (pick == 2)
        {
            do
            {
                //Displaying the Pho Menu
                cout << "       Pho - Rice Noodle Soup" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < phoMenu.size(); x++)
                {
                    cout << x+1 << ". " << setw(100) << left << phoMenu[x].itemName << "$" ;
                    cout << phoMenu[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Pho menu [enter between 1 - 5 and enter zero to exit this sub menu]?: ";
                cin >>answer;

                while (!( (answer > -1)  && (answer < 6) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 0-5: ";
                    cin >> answer;
                }

                if (answer > 0)
                {
                    
                    cout << "How many of " << phoMenu[answer - 1].itemName << "would you like?: ";
                    cin >> num1;
                    //vector<singleOrder> orderHistory;
                    orderHistory1.push_back(singleOrder((answer-1), num1));

                    cout << "Would you like to order something else from Pho menu?[Enter 1 for YES and 0 for No]: ";
                    cin >>answer;
                }

            }
            while (answer != 0);

        }
        if (pick == 3)
        {
            do
            {
            //Displaying the Egg Noodles Menu
                cout << "       Mi - Egg Noodles" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < eggNoodles.size(); x++)
                {
                    cout << x+1 << ". " << setw(110) << eggNoodles[x].itemName << "$" << setw(10);
                    cout << eggNoodles[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Egg Noodles' menu [enter between 1 - 5 and enter zero to exit this sub menu]?";
                cin >> answer;

                while (!( (answer > -1)  && (answer < 6) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 0-5";
                    cin >> answer;
                }

                if (answer > 0)
                {                
                    cout << "How many of " << eggNoodles[answer - 1].itemName << "would you like? ";
                    cin >> num1;

                    //vector<singleOrder> orderHistory;
                    orderHistory2.push_back(singleOrder((answer-1), num1));

                    cout << "Would you like to order something else from Egg Noodles menu?[Enter 1 for YES and 2 for No]: ";
                    cin >>answer;
                }
  
            }
            while (answer != 0);

        }

        if (pick == 4)
        {
            do
            {
            //Displaying the Vietnamese sandwich Menu
                cout << " Banh Mi (sandwich) Menu" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < banhMi.size(); x++)
                {
                    cout << x+1 << ". " << setw(75) << banhMi[x].itemName << "$" << setw(10);
                    cout << banhMi[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Appetizer menu [enter between 1 - 10 and enter zero to exit this sub menu]?";
                cin >> answer;

                while (!( (answer > -1)  && (answer < 5) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 0-4";
                    cin >> answer;
                }

                if (answer > 0)
                {                
                    cout << "How many of " << banhMi[answer - 1].itemName << "would you like? ";
                    cin >> num1;

                    //vector<singleOrder> orderHistory;
                    orderHistory3.push_back(singleOrder((answer-1), num1));

                    cout << "Would you like to order something else from Pho menu?[Enter 1 for YES and 0 for No]";
                    cin >>answer;
                }

            }
            while (answer != 0);

        }

        if (pick == 5)
        {
            do
            {
            //Displaying the Drinks Menu
                cout << " Drinks Menu" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < drinksMenu.size(); x++)
                {
                    cout << x+1 << ". " << setw(70) << drinksMenu[x].itemName << "$" << setw(10);
                    cout << drinksMenu[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Appetizer menu [enter between 1 - 10 and enter zero to exit this sub menu]?";
                cin >> answer;

                while (!( (answer > -1)  && (answer < 10) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 0-9";
                    cin >> answer;
                }
  
                if (answer > 0)
                {
                    cout << "How many of " << drinksMenu[answer - 1].itemName << "would you like? ";
                    cin >> num1;

                    //vector<singleOrder> orderHistory;
                    orderHistory4.push_back(singleOrder((answer-1), num1));

                    cout << "Would you like to order something else from Pho menu?[Enter 1 for YES and 2 for No]";
                    cin >>answer;

                }

            }
            while (answer != 0);

        }
 
        if (pick == 6)
        {
            double summingAmmount = 0;
            do{
                for (unsigned int i = 0; i < appetizersMenu.size(); i++)
                {
                    double totalSales = 0;
                    unsigned int totalQuantity = 0;
                    //calculating the total price per order
                    for ( unsigned int y  =0; y < orderHistory.size(); y++ )
                    {
                        if (orderHistory[y].itemOrdered == i)
                        {
                            totalQuantity += orderHistory[y].numberItems;
                        }
                    }
                    totalSales = totalQuantity * appetizersMenu[i].itemCost;

                    summingAmmount = summingAmmount + totalSales;
                    //print out only the orders that have been placed and the total price
                    if (totalQuantity > 0)
                        cout << setw(120) << left << appetizersMenu[i].itemName << setw(5) << left << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                            
                        //}
                        //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
                }
                for (unsigned int x = 0; x < phoMenu.size(); x++)
                {
                    double totalSales = 0;
                    unsigned int totalQuantity = 0;
                    //calculating the total price per order
                    for ( unsigned int j  =0; j < orderHistory1.size(); j++ )
                    {
                        if (orderHistory1[j].itemOrdered == x)
                        {
                            totalQuantity += orderHistory1[j].numberItems;
                        }
                    }
                    totalSales = totalQuantity * phoMenu[x].itemCost;

                    summingAmmount = summingAmmount + totalSales;
                    //print out only the orders that have been placed and the total price
                    if (totalQuantity > 0)
                        cout << setw(120) << left << phoMenu[x].itemName << setw(5) << left << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                        
                    //}
                    //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
                } 
                for (unsigned int x = 0; x < eggNoodles.size(); x++)
                {
                    double totalSales = 0;
                    unsigned int totalQuantity = 0;
                    //calculating the total price per order
                    for ( unsigned int j  =0; j < orderHistory2.size(); j++ )
                    {
                        if (orderHistory2[j].itemOrdered == x)
                        {
                            totalQuantity += orderHistory2[j].numberItems;
                        }
                    }
                    totalSales = totalQuantity * eggNoodles[x].itemCost;

                    summingAmmount = summingAmmount + totalSales;
                    //print out only the orders that have been placed and the total price
                    if (totalQuantity > 0)
                        cout << setw(120) << left << eggNoodles[x].itemName << setw(5) << left << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                        
                    //}
                    //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
                } 
                for (unsigned int x = 0; x < banhMi.size(); x++)
                {
                    double totalSales = 0;
                    unsigned int totalQuantity = 0;
                    //calculating the total price per order
                    for ( unsigned int j  =0; j < orderHistory3.size(); j++ )
                    {
                        if (orderHistory3[j].itemOrdered == x)
                        {
                            totalQuantity += orderHistory3[j].numberItems;
                        }
                    }
                    totalSales = totalQuantity * banhMi[x].itemCost;

                    summingAmmount = summingAmmount + totalSales;
                    //print out only the orders that have been placed and the total price
                    if (totalQuantity > 0)
                        cout << setw(120) << left << banhMi[x].itemName << setw(5) << left << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                        
                    //}
                    //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
                } 
                for (unsigned int x = 0; x < drinksMenu.size(); x++)
                {
                    double totalSales = 0;
                    unsigned int totalQuantity = 0;
                    //calculating the total price per order
                    for ( unsigned int j  =0; j < orderHistory4.size(); j++ )
                    {
                        if (orderHistory4[j].itemOrdered == x)
                        {
                            totalQuantity += orderHistory4[j].numberItems;
                        }
                    }
                    totalSales = totalQuantity * drinksMenu[x].itemCost;

                    summingAmmount = summingAmmount + totalSales;
                    //print out only the orders that have been placed and the total price
                    if (totalQuantity > 0)
                        cout << setw(120) << left << drinksMenu[x].itemName << setw(5) << left << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                        
                    //}
                    //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
                } 
                cout << endl;
                cout << endl;
                cout << setw(120) << left<< "Total amount is: " << summingAmmount <<endl;
                    
                cout << endl;
                cout << "Would you like to pay or return to the main menu? [Enter 1 to pay or Enter 0 to go back to the main menu: ";
                cin  >> answer;


                if (answer == 1)
                {
                    finalPayment(summingAmmount);
                    exit(0);
                }

                cout << endl;

            }while(answer != 0);
            
        }
    } 
   
} 