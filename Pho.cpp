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

//A flexible approach that would keep track of all the indendepent orders that way and then would generate a final report
struct singleOrder{
    singleOrder(unsigned int io, unsigned int ni): itemOrdered(io), numberItems(ni) {}
    unsigned int itemOrdered;
    unsigned int numberItems;

};

int mainMenu();
int finalPayment(double summingAmount);

int finalPayment( double summingAmount) {

    //double summingAmount;
    //checkins the credit card number entered as this payment method only accepts MasterCard, Visa, and American Express
    // To validate the credit card number used, we use the Luhn algorithm

    long number, checksum, checksum1, checksum2, checksum3, checksum4, checksum5, checksum6, checksum7;
    long checksum8, checksum9, checksum10, checksum11, checksum12, checksum13, checksum14, checksum15;

    int total, total1, check, check1, check2, check3, check4, check5, check6, check7, check8, check9; 
    int  check11, check10, check12, check13, check14, check15;

    cout << "Enter credit card number [only Visa, MAsterCard and American Express]: ";
    cin >> number;

    checksum   = number % 10;
    checksum1  = (number % 100) / 10;
    checksum2  = (number % 1000) / 100;
    checksum3  = (number % 10000) / 1000;
    checksum4  = (number % 100000) / 10000;
    checksum5  = (number % 1000000) / 100000;
    checksum6  = (number % 10000000) / 1000000;
    checksum7  = (number % 100000000) / 10000000;
    checksum8  = (number % 1000000000) / 100000000;
    checksum9  = (number % 10000000000) / 1000000000;
    checksum10 = (number % 100000000000) / 10000000000;
    checksum11 = (number % 1000000000000) / 100000000000; 
    checksum12 = (number % 10000000000000) / 1000000000000;
    checksum13 = (number % 100000000000000) / 10000000000000;
    checksum14 = (number % 1000000000000000) / 100000000000000;
    checksum15 = (number % 10000000000000000) / 1000000000000000;

    if (checksum14 == 3 & checksum13 == 7 || checksum14 == 3 & checksum13 == 4)  //if numbers start from 37 or 34 perform this operation
    {
        check13 = checksum13 * 2;  //if the condition is satisfied then perform this operation
        check11 = checksum11 * 2;
        check9  = checksum9 * 2;
        check7  = checksum7 * 2;
        check5  = checksum5 * 2;
        check3  = checksum3 * 2;
        check1  = checksum1 * 2;

        total   = (check1 / 10) + (check1 % 10) + (check3 / 10) + (check3 % 10) + (check5 / 10) + (check5 % 10) +
                  (check7 / 10) + (check7 % 10) + (check9 / 10) + (check9 % 10) + (check11 / 10) + (check11 % 10) + 
                  (check13 / 10) + (check13 % 10);  //multiplying the tows number by 2 and then adding the digits of the numbers individually
        total1  = total + checksum14 + checksum12 + checksum10 + checksum8 + checksum6 + checksum4 + checksum2 + checksum;
        //adding the other numbers with the total value
        if
        (total1 % 10 == 0)  //if the last digit of the total 1 is zero than it is amex
        {
            int one, two;
            for (int i = 0; i < 4; i++)
            {
                cout << ".";
                this_thread::sleep_for(1s);
            }
            cout << endl;
            cout << "ACCEPTED....  ";
            printf("AMEX\n");
            cout << "Would you like to leave a TIP? [1 for yes, 0 for no]";
            cin >> one;
            
            if (one == 1)
            {
                double newTotal, newTotal1, newTotal2;
                newTotal  = summingAmount + (0.18 *summingAmount);
                newTotal1 = summingAmount + (0.20 * summingAmount);
                newTotal2 = summingAmount + (0.24 * summingAmount);
                
                cout << "Would you like to pick 18% " << "[" << newTotal << "], " << "20%" << "[" << newTotal1 << "], " << "or ";
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
                
                
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    cout << ".";
                    this_thread::sleep_for(1s);
                }
                cout << "Thank you! Your payment of " << summingAmount <<" was accepted" << endl;  
            }
            
   
        }
        else
        {
            printf("INVALID\n");
        }

    }
    else if (checksum15 == 5 & checksum14 == 1 || checksum15 == 5 & checksum14 == 2 || checksum15 == 5 & checksum14 == 3 
             || checksum14 == 5 & checksum14 == 4 || checksum15 == 5 & checksum14 == 5)  //if number starts from 51, 52, 53, 54 or 55
    {
        check15 = checksum15 * 2;  //if it statisfies the condition then perform this operation
        check13 = checksum13 * 2;
        check11 = checksum11 * 2;
        check9  = checksum9 * 2;
        check7  = checksum7 * 2;
        check5  = checksum5 * 2;
        check3  = checksum3 * 2;
        check1  = checksum1 * 2;

        total   = (check15 / 10) + (check15 % 10) + (check13 / 10) + (check13 % 10) + (check11 / 10) + (check11 % 10) + (check9 / 10) + 
                  (check9 % 10) + (check7 / 10) + (check7 % 10) + (check5 / 10) + (check5 % 10) + (check3 % 10) + (check3 / 10) + (check1 / 10) + 
                  (check1 % 10); //multiplying the tows number by 2 and then adding the digits of the numbers individually

        total1  = total + checksum + checksum2 + checksum4 + checksum6 + checksum8 + checksum10 + checksum12 + checksum14;
        //adding the other numbers with the total value
        if
        (total1 % 10 == 0)  //if the last digit of the total is zero then its a mastercard
        {
            int one, two;
            for (int i = 0; i < 4; i++)
            {
                cout << ".";
                this_thread::sleep_for(1s);
            }
            cout << endl;
            cout << "ACCEPTED....  ";
            printf("MASTERCARD\n");

            cout << "Would you like to leave a TIP? [1 for yes, 0 for no] : ";
            cin >> one;
            
            if (one == 1)
            {
                double newTotal, newTotal1, newTotal2;
                newTotal  = summingAmount + (0.18 *summingAmount);
                newTotal1 = summingAmount + (0.20 * summingAmount);
                newTotal2 = summingAmount + (0.24 * summingAmount);
                
                cout << "Would you like to pick 18%" << "[" << newTotal << "], " << "20%" << "[" << newTotal1 << "], " << "or ";
                cout <<"24%" << "[" << newTotal2 << "]?" << "[enter 1-3 for the choices] : ";
                cin >> two;
                if (two == 1)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal <<" was accpted" << endl;  
                }
                else if (two == 2)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal1 <<" was accpted" << endl;  
                }
                else if (two == 3)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal2 <<" was accpted" << endl;  
                }
                
                
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    cout << ".";
                    this_thread::sleep_for(1s);
                }
                cout << "Thank you! your payment of " << summingAmount <<" was accpted" << endl;  
            }
            
        }
        else
        {
            printf("INVALID\n");
        }
    }
    else if 
    (checksum12 == 4) //if the credit card number starts with 4
    {
        check11 = checksum11 * 2;  //if the number starts with 5 then perform this operation
        check9  = checksum9 * 2;
        check7  = checksum7 * 2;
        check5  = checksum5 * 2;
        check3  = checksum3 * 2;
        check1  = checksum1 * 2;
        
        total   = (check11 / 10) + (check11 % 10) + (check9 % 10) + (check9 / 10) + (check7 % 10) + (check7 / 10) + 
                  (check5 / 10) + (check5 % 10) + (check3 % 10) + (check3 / 10) + (check1 % 10) + (check1 / 10);
                  //multiplying the tows number by 2 and then adding the digits of the numbers individually
        total1 = total + checksum + checksum2 + checksum4 + checksum6 + checksum8 + checksum10 + checksum12;
         //adding the other numbers with the total value
        if
        (total1 % 10 == 0)
        {
            int one, two;
            for (int i = 0; i < 4; i++)
            {
                cout << ".";
                this_thread::sleep_for(2s);
            }
            cout << endl;
            cout << "ACCEPTED....  ";
            printf("VISA\n");

            cout << "Would you like to leave a TIP? [1 for yes, 0 for no]";
            cin >> one;

            if (one == 1)
            {
                int newTotal, newTotal1, newTotal2;
                newTotal  = summingAmount + (0.18 *summingAmount);
                newTotal1 = summingAmount + (0.20 * summingAmount);
                newTotal2 = summingAmount + (0.24 * summingAmount);
                
                cout << "Would you like to pick 18%" << "[" << newTotal << "], " << "20%" << "[" << newTotal1 << "], " << "or ";
                cout <<"24%" << "[" << newTotal2 << "]?" << "[enter 1-3 for the choices] : ";
                cin >> two;
                if (two == 1)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal <<" was accpted" << endl;  
                }
                else if (two == 2)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal1 <<" was accpted" << endl;  
                }
                else if (two == 3)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal2 <<" was accpted" << endl;  
                }
                
                
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    cout << ".";
                    this_thread::sleep_for(1s);
                }
                cout << "Thank you! your payment of " << summingAmount <<" was accpted" << endl;  
            }
        }
        else
        {
            printf("INVALID\n");
        }

    }
        else if
    (checksum15 == 4)  //if the number starts with 4
    { 
        check15 = checksum15 * 2;   //if the number starts with 4 then perform this operation
        check13 = checksum13 * 2;
        check11 = checksum11 * 2;
        check9  = checksum9 * 2;
        check7  = checksum7 * 2;
        check5  = checksum5 * 2;
        check3  = checksum3 * 2;
        check1  = checksum1 * 2;

        total   = (check15 / 10) + (check15 % 10) + (check13 / 10) + (check13 % 10) + (check11 / 10) + (check11 % 10) + (check9 / 10) + 
                  (check9 % 10) + (check7 / 10) + (check7 % 10) + (check5 / 10) + (check5 % 10) + (check3 % 10) + (check3 / 10) + (check1 / 10) + 
                  (check1 % 10);
        //multiplying the tows number by 2 and then adding the digits of the numbers individually
        total1  = total + checksum + checksum2 + checksum4 + checksum6 + checksum8 + checksum10 + checksum12 + checksum14;
        //adding the other numbers with the total value
        if
        (total1 % 10 == 0) //if the last digit ends with 0 then the number is visa
        {
            int one, two;
            for (int i = 0; i < 4; i++)
            {
                cout << ".";
                this_thread::sleep_for(1s);
            }
            cout << endl;
            cout << "ACCEPTED....  ";
            printf("VISA\n");
            if (one == 1)
            {
                int newTotal, newTotal1, newTotal2;
                newTotal  = summingAmount + (0.18 *summingAmount);
                newTotal1 = summingAmount + (0.20 * summingAmount);
                newTotal2 = summingAmount + (0.24 * summingAmount);
                
                cout << "Would you like to pick 18%" << "[" << newTotal << "], " << "20%" << "[" << newTotal1 << "], " << "or ";
                cout <<"24%" << "[" << newTotal2 << "]?" << "(enter 1-3 for the choices) : ";
                cin >> two;
                if (two == 1)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal <<" was accpted" << endl;  
                }
                else if (two == 2)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal1 <<" was accpted" << endl;  
                }
                else if (two == 3)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        cout << ".";
                        this_thread::sleep_for(1s);
                    }
                    cout << "Thank you! your payment of " << newTotal2 <<" was accpted" << endl;  
                }
                
                
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    cout << ".";
                    this_thread::sleep_for(1s);
                }
                cout << "Thank you! your payment of " << summingAmount <<" was accpted" << endl;  
            }
            
        }
        else
        {
            printf("INVALID\n");  //if it does not then it is invalid
        }
    }
    else
    {
        printf("INVALID\n");
    }

    return 0;

}

int mainMenu() {
    int choice;
    //Listing out the main menu choices!

    cout << "Welcome to the main menu! ";
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
    cout << "   Enter 8 for check-out" << endl;
    cout << "Enter choice between 1 - 8: ";
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
    //system("CLS")
    //mainMenu();
    //cout << string(50, '\n');

    //Adding the Appetizers items to the menu!
    vector<menuItemAppetizers> appetizersMenu;
    appetizersMenu.push_back(menuItemAppetizers("Vietnamese Egg Rolls(2) - Groud pork, shrimp, carrots, taro, served with lettuce and a sweet, spicy sauce  ", 2.99));
    appetizersMenu.push_back(menuItemAppetizers("Vietnamese Spring Rolls(2) - Pork, shrimp, rice, noodles and mint leaves wrapped in rice paper with a peanut sauce  ", 4.99));
    appetizersMenu.push_back(menuItemAppetizers("Nem Nuong Cuon(2) - Grilled pork sausage spring roll  ", 4.99));
    appetizersMenu.push_back(menuItemAppetizers("Bo Nuong - Short rib beef marinated and grilled. Served with lettuce and cilantro  ", 7.99));
    appetizersMenu.push_back(menuItemAppetizers("Myo Chien - Lightly coated deep fried calamari, stir-tried with fresh garllic and chill pepper. Served with lettuce and cilantro  ", 8.99));
    appetizersMenu.push_back(menuItemAppetizers("Hen Nuog - Marinated mussels grilled with green onions  ", 9.99));
    appetizersMenu.push_back(menuItemAppetizers("Chim Cut Quay - quails slowly roasted to perfection  ", 9.99));
    appetizersMenu.push_back(menuItemAppetizers("Canh Ga Nudo Mam(6) - Stir fried chicken with fish sauce  ", 6.99));
    appetizersMenu.push_back(menuItemAppetizers("Seasame Chicken Wings(6) - Deep fried chicken wings cooked in sesame sauce  ", 6.99));
    appetizersMenu.push_back(menuItemAppetizers("House Chicken Wings(6) - Deep fried, spicy chicken wings stir-fried in our special garlic seasoning  ", 6.99));

    vector<menuItemRiceNoodleSoup> phoMenu;
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Dao Biet - Rice noodle soup served with sliced flank steak beef brisket tendom, and meat balls  ", 9.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Tai      - Rice noodle served with sliced flank steak  ", 8.99));
    phoMenu.push_back(menuItemRiceNoodleSoup("Pho Ga       - Rice noodle soup served with seasoned chicken breast  ", 8.99));


    vector<singleOrder> orderHistory;
    vector<singleOrder> orderHistory1;

            //cin.ignore();
    
    
    do
    //while(((pick > -1 ) && (pick < 9)))
    {
        //cin.ignore();
        //cout << endl;
        
        //cout << "Enter 0 to go to Main menu : ";
        //cin >> answer;
        //mainMenu();
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
                    cout << x+1 << ". " << setw(40) << appetizersMenu[x].itemName << "$" << setw(40);
                    cout << appetizersMenu[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Appetizer menu [enter between 1 - 10]?";
                cin >> order;

                while (!( (order > 0)  && (order < 11) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 1-10";
                    cin >> order;
                }
  

                cout << "How many of " << appetizersMenu[order - 1].itemName << " would you like? ";
                cin >> num1;

                //vector<singleOrder> orderHistory;
                orderHistory.push_back(singleOrder((order-1), num1));

                cout << "Would you like to order something else from appetizer menu?[Enter 1 for YES and 2 for No]";
                cin >>answer;
            }
            while (answer != 2);

        }
        if (pick == 2)
        {
            do
            {
            //Displaying the Appetizer Menu
                cout << "       Pho - Rice Noodle Soup" << endl;
                cout << endl;
                cout.setf(ios::left);  //makes the field left justified
                for (unsigned int x =0; x < phoMenu.size(); x++)
                {
                    cout << x+1 << ". " << setw(40) << phoMenu[x].itemName << "$" << setw(10);
                    cout << phoMenu[x].itemCost << endl;
                }

            //cout << endl;
                cout << endl;
                cout << "What would you like to order from the Appetizer menu [enter between 1 - 10]?";
                cin >> order;

                while (!( (order > 0)  && (order < 4) ) )
                {
                    cout << "Invalid order number! Enter a valid order number between 1-10";
                    cin >> order;
                }
  

                cout << "How many of " << phoMenu[order - 1].itemName << " would you like? ";
                cin >> num1;

                //vector<singleOrder> orderHistory;
                orderHistory1.push_back(singleOrder((order-1), num1));

                cout << "Would you like to order something else from Pho menu?[Enter 1 for YES and 2 for No]";
                cin >>answer;
            }
            while (answer != 2);

        }
 

        if (pick == 8)
        {
            double summingAmmount = 0;
            //do{
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
                    cout << setw(20) << appetizersMenu[i].itemName << setw(5) << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                    
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
                    cout << setw(20) << phoMenu[x].itemName << setw(5) << totalQuantity << "$" << totalSales << "    " << summingAmmount << endl;
                    
                //}
                //cout << "Would you like to go back to the main menu?[Enter 1 for YES and 2 for NO";
            } 
            
            cout << endl;
            cout << "Would you like to pay or return to the main menu? [Enter 1 to pay or Enter 0 to go back to the main menu: ";
            cin  >> choice;


            if (choice == 1)
            {
                finalPayment(summingAmmount);
            }

            exit(0);

            //finalPayment(summingAmmount);


            
            //while (answer != 1);

            
        }

        //cout << "Would like to got back to the main menu or check out?[Enter 0]";
        //cin >>  choice;
       // pick = mainMenu(answer);

    } while(((pick > -1 ) && (pick < 9)));
    cout << "error";
    
   
} 