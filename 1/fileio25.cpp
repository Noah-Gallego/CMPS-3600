// Noah Gallego

#include <iostream>
#include <fstream>
using namespace std;

int main() { 
    // Collect Name from User
    string name;
    cout << "Enter your name: ";
    getline(cin, name);
    cout << "Your name is: " << name << "." << endl;

    // Get Number from User
    int num;
    cout << "Enter a number: ";
    cin >> num;
    cout << "You number is " << num << "." << endl;

    // Open 'Log' File
    ofstream fout("log");
    
    // Start Summation
    int sum;
    for (int i = 1; i < num; i++) sum += i;
    cout << "The sum from 1 to " << num << " is " << sum << "." << endl;

    // Close the 'Log' File
    fout << "The summation is: " << sum << "." << endl;

    return 0;
}
