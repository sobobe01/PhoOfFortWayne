//This program demonstrates the searchList function, which
//performs a linear search on an integer array.

#include <iostream>
#include <sstream>
using namespace std;
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>

int main(void)
{
    //long total, total1;

    //string number;
    
    //int count = 0;

    string number;
    //vector<int>creditCard;
    
    cout << "Enter a Credit Card number: ";
    cin >> number;
    //getline(cin, number);
    //istringstream iss(number);
   // cin >> number;
    
    if ( (number[0] - '0' == 3 & number[1] - '0' == 7) || (number[0] - '0' == 3 & number[1] - '0' == 4) )
    {
        long total = 0;
        for (int i = 0; i < 7; i++)
        { 
            total = total + (( ( (number[2*i+1] - '0') * 2) / 10) + ( ( (number[2*i+1] - '0') * 2) % 10));
            //total = total + (( ( (number[2*i+1]) * 2) / 10) + ( ( (number[2*i+1]) * 2) % 10));
            cout << total << endl;
        }

        long total1 = total;
        for (int x = 0; x < 8; x++)
        {
            total1 =total1 + (number[2*x] - '0');
            //cout << total1 << endl;
        } 
        
        if (total1 % 10 == 0)  //if the last digit of the total 1 is zero than it is amex
        {
            printf("AMEX\n");
        }
        else
        {
            printf("INVALID\n");
        }
    }

    else if ( number[0] - '0' == 5 & (number[1] - '0' == 1 || number[1] - '0' == 2 || number[1] - '0' == 3 || number[1] - '0' == 4 || number[1] - '0' == 5))
    //(number[0] - '0' == 5 & number[1] - '0' == 5) || (number[0] - '0' == 2 & number[0] - '0' == 2) || (number[0] - '0' == 2 & number[0] - '0'== 7))
    {
        long total = 0;
        for (int i = 0; i < 8; i++)
        { 
            total = total + (( ( (number[2*i+1] - '0') * 2) / 10) + ( ( (number[2*i+1] - '0') * 2) % 10));

            cout << ( ( (number[2*i+1] - '0') * 2) / 10) << " " <<( ( (number[2*i+1] - '0') * 2) % 10) << " " << total << endl;
            //cout << ( ( (number[2*i+1] - '0') * 2) % 10) << " " << endl;

            //cout << total << endl;
        }
        cout << endl;

        long total1 = total;
        for (int x = 0; x < 8; x++)
        {
            total1 =total1 + (number[2*x] - '0');
            cout << total1 << endl;
        }

        //int total2 = total1 + total;

        if (total1 % 10 == 0)  //if the last digit of the total is zero then its a mastercard
        {
            printf("MASTERCARD\n");
        }
        else
        {
            printf("INVALID\n");
        }
    }

    cout << "NOT WORKING" << endl;

}

