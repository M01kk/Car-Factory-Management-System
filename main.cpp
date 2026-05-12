#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>

using namespace std;

// CONSTANTS (array max sizes and overflow protection)
const int MAX_USERS = 100;
const int MAX_CARS = 100;
const int MAX_CUSTOMERS = 100;
const int MAX_REQUESTS = 100;
const int USERNAME_LEN = 11;   /* 10 chars + null */
const int PASSWORD_LEN = 9;    /* 8 chars + null */
const int BRAND_LEN = 51;
const int MODEL_LEN = 51;
const int COLOR_LEN = 11;
const int PASSPORT_LEN = 21;

/* Role: 0 = Employee, 1 = CEO */
const int ROLE_EMPLOYEE = 0;
const int ROLE_CEO = 1;

/* Color codes: 0=White, 1=Black, 2=Gray */
const int COLOR_WHITE = 0;
const int COLOR_BLACK = 1;
const int COLOR_GRAY = 2;

/* Request status: 0=Pending, 1=Accepted, 2=Rejected */
const int REQ_PENDING = 0;
const int REQ_ACCEPTED = 1;
const int REQ_REJECTED = 2;

// DATA STRUCTURES  

struct User {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    int role;  //ROLE_EMPLOYEE or ROLE_CEO 
};

struct Car {
    int id;
    char brand[BRAND_LEN];
    char model[MODEL_LEN];
    int year;
    int color;  /* COLOR_WHITE, COLOR_BLACK, COLOR_GRAY */
    double price;
    int quantity;
};

struct Customer {
    char username[USERNAME_LEN];
    char password[PASSPORT_LEN];  /* second field stored in file */
};

struct PurchaseRequest {
    char customerUsername[USERNAME_LEN];
    char brand[BRAND_LEN];
    char model[MODEL_LEN];
    int year;
    int color;
    int status;  /* REQ_PENDING, REQ_ACCEPTED, REQ_REJECTED */
};

// GLOBAL DATA
User users[MAX_USERS];
int userCount = 0;

Car cars[MAX_CARS];
int carCount = 0;

Customer customers[MAX_CUSTOMERS];
int customerCount = 0;

PurchaseRequest requests[MAX_REQUESTS];
int requestCount = 0;

int currentUserIndex = -1;  /* -1 means not logged in */

// FILE NAMES 
const char FILE_USERS[] = "users.txt";
const char FILE_CARS[] = "cars.txt";
const char FILE_CUSTOMERS[] = "customers.txt";
const char FILE_REQUESTS[] = "requests.txt";

//HELPER: get color name from code 
void colorToString(int color, char* out) {
    if (color == COLOR_WHITE) strcpy(out, "White");
    else if (color == COLOR_BLACK) strcpy(out, "Black");
    else if (color == COLOR_GRAY) strcpy(out, "Gray");
    else strcpy(out, "Unknown");
}

// HELPER: parse color from string 
int stringToColor(const char* s) {
    if (strcmp(s, "White") == 0) return COLOR_WHITE;
    if (strcmp(s, "Black") == 0) return COLOR_BLACK;
    if (strcmp(s, "Gray") == 0) return COLOR_GRAY;
    return -1;
}

//VALIDATION: username (max 10 chars) 
int isValidUsername(const char* username) {
    int len = strlen(username);
    if (len == 0 || len > 10) return 0;
    for (int i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') return 0;
    }
    return 1;
}

// VALIDATION: password (exactly 8 chars, letters and numbers) 
int isValidPassword(const char* password) {
    if (strlen(password) != 8) return 0;
    int hasLetter = 0, hasDigit = 0;
    for (int i = 0; i < 8; i++) {
        if (isalpha(password[i])) hasLetter = 1;
        if (isdigit(password[i])) hasDigit = 1;
    }
    return (hasLetter && hasDigit);
}

//Check if username already exists 
int findUserByUsername(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) return i;
    }
    return -1;
}

//Check if car ID exists 
int findCarById(int id) {
    for (int i = 0; i < carCount; i++) {
        if (cars[i].id == id) return i;
    }
    return -1;
}

// Check if customer username exists  
int findCustomerByUsername(const char* username) {
    for (int i = 0; i < customerCount; i++) {
        if (strcmp(customers[i].username, username) == 0) return i;
    }
    return -1;
}

// LOAD DATA FROM FILES  
void loadUsers() {
    ifstream f(FILE_USERS);
    userCount = 0;
    if (!f.is_open()) return;
    char line[200];
    while (f.getline(line, 199) && userCount < MAX_USERS) {
        char u[USERNAME_LEN], p[PASSWORD_LEN];
        int r;
        if (sscanf(line, "%10s %8s %d", u, p, &r) == 3) {
            strcpy(users[userCount].username, u);
            strcpy(users[userCount].password, p);
            users[userCount].role = r;
            userCount++;
        }
    }
    f.close();
}

void loadCars() {
    ifstream f(FILE_CARS);
    carCount = 0;
    if (!f.is_open()) return;
    char line[300];
    while (f.getline(line, 299) && carCount < MAX_CARS) {
        Car c;
        char col[COLOR_LEN];
        if (sscanf(line, "%d %50s %50s %d %10s %lf %d",
                   &c.id, c.brand, c.model, &c.year, col, &c.price, &c.quantity) == 7) {
            c.color = stringToColor(col);
            if (c.color >= 0) {
                cars[carCount] = c;
                carCount++;
            }
        }
    }
    f.close();
}

void loadCustomers() {
    ifstream f(FILE_CUSTOMERS);
    customerCount = 0;
    if (!f.is_open()) return;
    char line[200];
    while (f.getline(line, 199) && customerCount < MAX_CUSTOMERS) {
        char u[USERNAME_LEN], p[PASSPORT_LEN];
        if (sscanf(line, "%10s %20[^\n]", u, p) >= 1) {
            strcpy(customers[customerCount].username, u);
            if (sscanf(line, "%10s %20s", u, p) >= 2)
                strcpy(customers[customerCount].password, p);
            else
                customers[customerCount].password[0] = '\0';
            customerCount++;
        }
    }
    f.close();
}

void loadRequests() {
    ifstream f(FILE_REQUESTS);
    requestCount = 0;
    if (!f.is_open()) return;
    char line[300];
    while (f.getline(line, 299) && requestCount < MAX_REQUESTS) {
        PurchaseRequest r;
        char col[COLOR_LEN];
        if (sscanf(line, "%10s %50s %50s %d %10s %d",
                   r.customerUsername, r.brand, r.model, &r.year, col, &r.status) == 6) {
            r.color = stringToColor(col);
            if (r.color >= 0) {
                requests[requestCount] = r;
                requestCount++;
            }
        }
    }
    f.close();
}

void loadAllData() {
    loadUsers();
    loadCars();
    loadCustomers();
    loadRequests();
}

// SAVE DATA TO FILES  
void saveUsers() {
    ofstream f(FILE_USERS);
    if (!f.is_open()) return;
    for (int i = 0; i < userCount; i++) {
        f << users[i].username << " " << users[i].password << " " << users[i].role << "\n";
    }
    f.close();
}

void saveCars() {
    ofstream f(FILE_CARS);
    if (!f.is_open()) return;
    char col[COLOR_LEN];
    for (int i = 0; i < carCount; i++) {
        colorToString(cars[i].color, col);
        f << cars[i].id << " " << cars[i].brand << " " << cars[i].model << " "
          << cars[i].year << " " << col << " " << cars[i].price << " " << cars[i].quantity << "\n";
    }
    f.close();
}

void saveCustomers() {
    ofstream f(FILE_CUSTOMERS);
    if (!f.is_open()) return;
    for (int i = 0; i < customerCount; i++) {
        f << customers[i].username << " " << customers[i].password << "\n";
    }
    f.close();
}

void saveRequests() {
    ofstream f(FILE_REQUESTS);
    if (!f.is_open()) return;
    char col[COLOR_LEN];
    for (int i = 0; i < requestCount; i++) {
        colorToString(requests[i].color, col);
        f << requests[i].customerUsername << " " << requests[i].brand << " " << requests[i].model << " "
          << requests[i].year << " " << col << " " << requests[i].status << "\n";
    }
    f.close();
}

// Initialize predefined users if file empty  
void initPredefinedUsers() {
    if (userCount > 0) return;
    if (userCount >= MAX_USERS) return;
    strcpy(users[0].username, "Mobina99");
    strcpy(users[0].password, "12345a");
    users[0].role = ROLE_CEO;
    userCount++;

    if (userCount >= MAX_USERS) return;
    strcpy(users[1].username, "Ghasem12");
    strcpy(users[1].password, "789mn11");
    users[1].role = ROLE_EMPLOYEE;
    userCount++;

    if (userCount >= MAX_USERS) return;
    strcpy(users[2].username, "Nori242");
    strcpy(users[2].password, "1278Gy0");
    users[2].role = ROLE_EMPLOYEE;
    userCount++;

    saveUsers();
}

// AUTHENTICATION MENU  
void doLogin() {
    char username[USERNAME_LEN], password[PASSWORD_LEN];
    cout << "\n--- Log In ---\n";
    cout << "Username (max 10 characters): ";
    cin >> username;
    if (!isValidUsername(username)) {
        cout << "Error: Username must be 1-10 characters (letters, numbers, underscore).\n";
        return;
    }
    cout << "Password (exactly 8 characters, letters and numbers): ";
    cin >> password;
    int idx = findUserByUsername(username);
    if (idx < 0) {
        cout << "Error: User not found.\n";
        return;
    }
    // Check password match (format validation only for new registration)
    if (strcmp(users[idx].password, password) != 0) {
        cout << "Error: Wrong password.\n";
        return;
    }
    currentUserIndex = idx;
    cout << "Logged in as " << users[idx].username << ".\n";
}

void doRegister() {
    if (userCount >= MAX_USERS) {
        cout << "Error: Maximum number of users reached. Cannot register.\n";
        return;
    }
    char username[USERNAME_LEN], password[PASSWORD_LEN];
    cout << "\n--- Register ---\n";
    cout << "Username (max 10 characters, no duplicates): ";
    cin >> username;
    if (!isValidUsername(username)) {
        cout << "Error: Username must be 1-10 characters (letters, numbers, underscore).\n";
        return;
    }
    if (findUserByUsername(username) >= 0) {
        cout << "Error: This username already exists.\n";
        return;
    }
    cout << "Password (exactly 8 characters, must contain letters and numbers): ";
    cin >> password;
    if (!isValidPassword(password)) {
        cout << "Error: Password must be exactly 8 characters and contain BOTH letters and numbers.\n";
        return;
    }
    strcpy(users[userCount].username, username);
    strcpy(users[userCount].password, password);
    users[userCount].role = ROLE_EMPLOYEE;
    userCount++;
    saveUsers();
    cout << "Registration successful.\n";
}

void doLogout() {
    if (currentUserIndex < 0) {
        cout << "You are not logged in.\n";
        return;
    }
    cout << "Logged out.\n";
    currentUserIndex = -1;
}

void authMenu() {
    int choice;
    do {
        cout << "\n====================================\n";
        cout << "            HIBILI\n";
        cout << "    Car Factory Management\n";
        cout << "====================================\n";
        cout << "1. Log in\n";
        cout << "2. Log up (Register)\n";
        cout << "3. Exit\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        switch (choice) {
            case 1:
                doLogin();
                if (currentUserIndex >= 0) return;  /* go to main menu */
                break;
            case 2: doRegister(); break;
            case 3: cout << "Goodbye.\n"; exit(0); //خروج از برنامه
            default: cout << "Invalid option.\n";
        }
    } while (1);
}

 //CARS MENU  
void showCars() {
    cout << "\n--- Show Cars ---\n";
    if (carCount == 0) {
        cout << "No cars in database.\n";
        return;
    }
    char col[COLOR_LEN];
    for (int i = 0; i < carCount; i++) {
        colorToString(cars[i].color, col);
        cout << "ID: " << cars[i].id << " | " << cars[i].brand << " " << cars[i].model
             << " | Year: " << cars[i].year << " | Color: " << col
             << " | Price: " << cars[i].price << " | Qty: " << cars[i].quantity << "\n";
    }
}

void searchCars() {
    cout << "\n--- Search Cars ---\n";
    cout << "1. By Brand\n";
    cout << "2. By Model\n";
    cout << "3. By Year\n";
    cout << "4. By Color\n";
    cout << "5. Back\n";
    int opt;
    cout << "Choice: ";
    cin >> opt;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    if (opt == 5) return;

    char col[COLOR_LEN];
    char colStr[COLOR_LEN];
    int found = 0;
    if (opt == 1) {
        cout << "Enter brand: ";
        cin >> col;
        for (int i = 0; i < carCount; i++) {
            if (strcmp(cars[i].brand, col) == 0) {
                colorToString(cars[i].color, colStr);
                cout << "ID: " << cars[i].id << " | " << cars[i].brand << " " << cars[i].model
                     << " | Year: " << cars[i].year << " | Color: " << colStr
                     << " | Price: " << cars[i].price << " | Qty: " << cars[i].quantity << "\n";
                found++;
            }
        }
    } else if (opt == 2) {
        cout << "Enter model: ";
        cin >> col;
        for (int i = 0; i < carCount; i++) {
            if (strcmp(cars[i].model, col) == 0) {
                colorToString(cars[i].color, colStr);
                cout << "ID: " << cars[i].id << " | " << cars[i].brand << " " << cars[i].model
                     << " | Year: " << cars[i].year << " | Color: " << colStr
                     << " | Price: " << cars[i].price << " | Qty: " << cars[i].quantity << "\n";
                found++;
            }
        }
    } else if (opt == 3) {
        int y;
        cout << "Enter year: ";
        cin >> y;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
        for (int i = 0; i < carCount; i++) {
            if (cars[i].year == y) {
                colorToString(cars[i].color, colStr);
                cout << "ID: " << cars[i].id << " | " << cars[i].brand << " " << cars[i].model
                     << " | Year: " << cars[i].year << " | Color: " << colStr
                     << " | Price: " << cars[i].price << " | Qty: " << cars[i].quantity << "\n";
                found++;
            }
        }
    } else if (opt == 4) {
        cout << "Color (White, Black, Gray): ";
        cin >> colStr;
        int c = stringToColor(colStr);
        if (c < 0) {
            cout << "Invalid color.\n";
            return;
        }
        for (int i = 0; i < carCount; i++) {
            if (cars[i].color == c) {
                colorToString(cars[i].color, colStr);
                cout << "ID: " << cars[i].id << " | " << cars[i].brand << " " << cars[i].model
                     << " | Year: " << cars[i].year << " | Color: " << colStr
                     << " | Price: " << cars[i].price << " | Qty: " << cars[i].quantity << "\n";
                found++;
            }
        }
    } else {
        cout << "Invalid option.\n";
        return;
    }
    cout << "Found: " << found << " car(s).\n";
}

void addCar() {
    if (carCount >= MAX_CARS) {
        cout << "Error: Maximum cars reached. Cannot add more.\n";
        return;
    }
    cout << "\n--- Add Car ---\n";
    Car c;
    cout << "ID (must be > 100): ";
    cin >> c.id;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    if (c.id <= 100) {
        cout << "Error: ID must be greater than 100.\n";
        return;
    }
    if (findCarById(c.id) >= 0) {
        cout << "Error: This ID already exists.\n";
        return;
    }
    cout << "Brand: ";
    cin >> c.brand;
    cout << "Model: ";
    cin >> c.model;
    cout << "Year: ";
    cin >> c.year;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    cout << "Color (White, Black, Gray): ";
    char col[COLOR_LEN];
    cin >> col;
    c.color = stringToColor(col);
    if (c.color < 0) {
        cout << "Error: Invalid color. Use White, Black, or Gray.\n";
        return;
    }
    cout << "Price: ";
    cin >> c.price;
    cout << "Quantity: ";
    cin >> c.quantity;
    if (cin.fail() || c.quantity < 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Error: Invalid quantity.\n";
        return;
    }
    cars[carCount] = c;
    carCount++;
    saveCars();
    cout << "Car added successfully.\n";
}

void updateCar() {
    cout << "\n--- Update Car ---\n";
    int id;
    cout << "Enter car ID: ";
    cin >> id;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    int idx = findCarById(id);
    if (idx < 0) {
        cout << "Car not found.\n";
        return;
    }
    cout << "New price: ";
    cin >> cars[idx].price;
    cout << "New quantity: ";
    cin >> cars[idx].quantity;
    if (cin.fail() || cars[idx].quantity < 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid quantity.\n";
        return;
    }
    cout << "New color (White, Black, Gray): ";
    char col[COLOR_LEN];
    cin >> col;
    int c = stringToColor(col);
    if (c >= 0) cars[idx].color = c;
    saveCars();
    cout << "Car updated successfully.\n";
}

void deleteCar() {
    cout << "\n--- Delete Car ---\n";
    int id;
    cout << "Enter car ID to delete: ";
    cin >> id;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    int idx = findCarById(id);
    if (idx < 0) {
        cout << "Car not found.\n";
        return;
    }
    cout << "Are you sure you want to delete this car? (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "Delete cancelled.\n";
        return;
    }
    for (int i = idx; i < carCount - 1; i++) {
        cars[i] = cars[i + 1];
    }
    carCount--;
    saveCars();
    cout << "Car deleted successfully.\n";
}

void carsMenu() {
    int choice;
    do {
        cout << "\n====================================\n";
        cout << "           CARS MENU\n";
        cout << "====================================\n";
        cout << "1. Show Cars\n";
        cout << "2. Search\n";
        cout << "3. Add Car\n";
        cout << "4. Update Car\n";
        cout << "5. Delete Car\n";
        cout << "6. Back\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        switch (choice) {
            case 1: showCars(); break;
            case 2: searchCars(); break;
            case 3: addCar(); break;
            case 4:
                if (currentUserIndex >= 0 && users[currentUserIndex].role != ROLE_CEO) {
                    cout << "Access Denied.\n";
                } else {
                    updateCar();
                }
                break;
            case 5:
                if (currentUserIndex >= 0 && users[currentUserIndex].role != ROLE_CEO) {
                    cout << "Access Denied.\n";
                } else {
                    deleteCar();
                }
                break;
            case 6: return;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 6);
}

//SALES AND CUSTOMERS MENU  
void addCustomer() {
    if (customerCount >= MAX_CUSTOMERS) {
        cout << "Error: Maximum customers reached.\n";
        return;
    }
    cout << "\n--- Add Customer ---\n";
    cout << "Username: ";
    cin >> customers[customerCount].username;
    cout << "Password: ";
    cin >> customers[customerCount].password;
    customerCount++;
    saveCustomers();
    cout << "Customer added successfully.\n";
}

void addPurchaseRequest() {
    if (requestCount >= MAX_REQUESTS) {
        cout << "Error: Maximum requests reached.\n";
        return;
    }
    cout << "\n--- Add Purchase Request ---\n";
    cout << "Customer username: ";
    cin >> requests[requestCount].customerUsername;
    cout << "Brand: ";
    cin >> requests[requestCount].brand;
    cout << "Model: ";
    cin >> requests[requestCount].model;
    cout << "Year: ";
    cin >> requests[requestCount].year;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    cout << "Color (White, Black, Gray): ";
    char col[COLOR_LEN];
    cin >> col;
    requests[requestCount].color = stringToColor(col);
    if (requests[requestCount].color < 0) {
        cout << "Invalid color.\n";
        return;
    }
    requests[requestCount].status = REQ_PENDING;
    requestCount++;
    saveRequests();
    cout << "Purchase request added successfully.\n";
}

void salesAndCustomersMenu() {
    int choice;
    do {
        cout << "\n====================================\n";
        cout << "     SALES AND CUSTOMERS\n";
        cout << "====================================\n";
        cout << "1. Add Customer\n";
        cout << "2. Add Purchase Request\n";
        cout << "3. Back\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        switch (choice) {
            case 1:
                if (currentUserIndex >= 0 && users[currentUserIndex].role != ROLE_CEO) {
                    cout << "Access Denied.\n";
                } else {
                    addCustomer();
                }
                break;
            case 2: addPurchaseRequest(); break;
            case 3: return;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 3);
}

//EMPLOYEES MENU  
void showRequestsSubMenu(int reqIndex) {
    int choice;
    do {
        cout << "\n--- Request #" << (reqIndex + 1) << " ---\n";
        char col[COLOR_LEN];
        colorToString(requests[reqIndex].color, col);
        cout << "Customer: " << requests[reqIndex].customerUsername
             << " | " << requests[reqIndex].brand << " " << requests[reqIndex].model
             << " | Year: " << requests[reqIndex].year << " | Color: " << col << "\n";
        cout << "1. Accept Request\n";
        cout << "2. Reject Request\n";
        cout << "3. Back\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        if (choice == 1) {
            requests[reqIndex].status = REQ_ACCEPTED;
            saveRequests();
            cout << "Request accepted.\n";
            return;
        } else if (choice == 2) {
            for (int i = reqIndex; i < requestCount - 1; i++)
                requests[i] = requests[i + 1];
            requestCount--;
            saveRequests();
            cout << "Request rejected and removed.\n";
            return;
        } else if (choice == 3) return;
        else cout << "Invalid option.\n";
    } while (1);
}

void showRequests() {
    cout << "\n--- Show Requests ---\n";
    if (requestCount == 0) {
        cout << "No requests.\n";
        return;
    }
    char col[COLOR_LEN];
    for (int i = 0; i < requestCount; i++) {
        colorToString(requests[i].color, col);
        cout << (i + 1) << ". " << requests[i].customerUsername << " | "
             << requests[i].brand << " " << requests[i].model << " | "
             << requests[i].year << " | " << col << "\n";
    }
    cout << "Select request number (0 = Back): ";
    int num;
    cin >> num;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); return; }
    if (num == 0) return;
    if (num < 1 || num > requestCount) {
        cout << "Invalid number.\n";
        return;
    }
    showRequestsSubMenu(num - 1);
}

void customersInformation() {
    cout << "\n--- Customers Information ---\n";
    cout << "Enter username: ";
    char uname[USERNAME_LEN];
    cin >> uname;
    int idx = findCustomerByUsername(uname);
    if (idx < 0) {
        cout << "Customer not found.\n";
        return;
    }
    cout << "Username: " << customers[idx].username << "\n";
    cout << "Password: " << customers[idx].password << "\n";
    int count = 0;
    cout << "Requests:\n";
    for (int i = 0; i < requestCount; i++) {
        if (strcmp(requests[i].customerUsername, uname) == 0) {
            count++;
            char col[COLOR_LEN];
            colorToString(requests[i].color, col);
            cout << "  " << count << ". " << requests[i].brand << " " << requests[i].model
                 << " | " << requests[i].year << " | " << col << "\n";
        }
    }
    if (count == 0) cout << "  (none)\n";
}

void employeesMenu() {
    int choice;
    do {
        cout << "\n====================================\n";
        cout << "         EMPLOYEES MENU\n";
        cout << "====================================\n";
        cout << "1. Show Requests\n";
        cout << "2. Customers Information\n";
        cout << "3. Back\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        switch (choice) {
            case 1: showRequests(); break;
            case 2: customersInformation(); break;
            case 3: return;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 3);
}

//REPORT (CEO ONLY)  
void reportMenu() {
    if (currentUserIndex < 0) return;
    if (users[currentUserIndex].role != ROLE_CEO) {
        cout << "Access denied. Report is for CEO only.\n";
        return;
    }
    cout << "\n====================================\n";
    cout << "           REPORT (CEO)\n";
    cout << "====================================\n";
    int totalSales = 0;
    for (int i = 0; i < requestCount; i++)
        if (requests[i].status == REQ_ACCEPTED) totalSales++;
    cout << "Total Sales (accepted requests): " << totalSales << "\n";
    cout << "Total Cars: " << carCount << "\n";
    cout << "Total Customers: " << customerCount << "\n";
    int empCount = 0;
    for (int i = 0; i < userCount; i++)
        if (users[i].role == ROLE_EMPLOYEE) empCount++;
    cout << "Total Employees: " << empCount << "\n";
    cout << "Total Requests: " << requestCount << "\n";
}

// MAIN MENU (after login)  
void mainMenu() {
    int choice;
    do {
        cout << "\n====================================\n";
        cout << "         MAIN MENU\n";
        cout << "====================================\n";
        cout << "1. Cars\n";
        cout << "2. Sales and Customers\n";
        cout << "3. Employees\n";
        cout << "4. Report (CEO only)\n";
        cout << "5. Logout\n";
        cout << "Choice: ";
        cin >> choice;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }
        switch (choice) {
            case 1: carsMenu(); break;
            case 2: salesAndCustomersMenu(); break;
            case 3:
                if (currentUserIndex >= 0 && users[currentUserIndex].role != ROLE_CEO) {
                    cout << "Access Denied.\n";
                } else {
                    employeesMenu();
                }
                break;
            case 4: reportMenu(); break;
            case 5:
                doLogout();
                return;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 5);
}
int main() {
    loadAllData();
    initPredefinedUsers();

    while (1) {
        authMenu();
        if (currentUserIndex < 0) continue;  /* not logged in, stay at auth or exited */
        while (currentUserIndex >= 0) {
            mainMenu();
        }
    }
    return 0;
}
