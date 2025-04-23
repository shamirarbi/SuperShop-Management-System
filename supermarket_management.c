#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>  // For getch() on Windows
#include <time.h>  // For time handling

#define MAX_UNDO 10
#define ADMIN_USER "admin"
#define ADMIN_PASS "1234"
#define MAX_USERS 10

// Function prototypes
void freeCustomers(); // Prototype for freeCustomers
void freeProducts(); // Prototype for freeProducts
void saveCustomers(); // Prototype for saveCustomers
void loadCustomers(); // Prototype for loadCustomers
void saveSellHistory(); // Prototype for saveSellHistory
void loadSellHistory(); // Prototype for loadSellHistory
void editPremiumThreshold(); // Prototype for editPremiumThreshold
void resetSellHistory();
void sellHistoryManagement();
void showAllUsers();
void customerInfoPanel();
void manageProductsPanel();
void menu();
int login(); // Prototype for login
void addCustomer();
void displayCustomers();
void removeCustomer();
void addProduct();
void updateProduct();
void deleteProduct();
void displayProducts();
void adminPanel();
float getFloatInput(const char* prompt);
int getIntInput(const char* prompt);
void addUser();
void removeUser();
void changeUserCredentials();
void undoDelete(); // Prototype for undoDelete
void searchProduct(); // Prototype for searchProduct
void searchByCategory(); // Prototype for searchByCategory
void sortByPrice(); // Prototype for sortByPrice
void showLowStock(); // Prototype for showLowStock
void clearAllStocks(); // Prototype for clearAllStocks
void clearAllProducts(); // Prototype for clearAllProducts
void managePremiumCustomers(); // Prototype for managePremiumCustomers
void addPremiumCustomer(); // Prototype for addPremiumCustomer
void removePremiumCustomer(); // Prototype for removePremiumCustomer
void displayPremiumCustomers(); // Prototype for displayPremiumCustomers
void editPremiumBuyAmount(); // Prototype for editPremiumBuyAmount
void discountManagement(); // Prototype for discountManagement
void showDailySellHistory(); // Prototype for showDailySellHistory
void showSellHistoryForPeriod(); // Prototype for showSellHistoryForPeriod
void saveUsers(); // Prototype for saveUsers
void loadUsers(); // Prototype for loadUsers
void updateStock(); // Prototype for updateStock
void searchCustomer(); // Prototype for searchCustomer

// Structure to store a product
typedef struct Product {
    char id[20];
    char name[50];
    char category[30];
    float price;
    int quantity;
    struct Product* next;
} Product;

// Structure for customer data
typedef struct Customer {
    char id[20];       // Unique customer ID
    char name[50];     // Customer name
    char phone[15];    // Customer phone number
    float totalSpent;  // Total amount spent by the customer
    int premium;       // 1 if the customer is premium, 0 otherwise
    struct {
        char id[20];
        char name[50];
        int quantity;
        float price;
    } purchases[50];   // Array to store purchase history
    int purchaseCount; // Number of purchases
    struct Customer* next;  // Pointer to the next customer (for linked list)
} Customer;

// Structure for user login
typedef struct {
    char username[20];
    char password[20];
    char role[10];  // admin, manager, etc.
} User;

// Structure for product data
typedef struct {
    char id[20];
    char name[50];
    char category[30];
    float price;
    int quantity;
} ProductData;

// Structure for sell record
typedef struct SellRecord {
    char customerId[20];
    char customerName[50];
    char productId[20];
    char productName[50];
    int quantity;
    float price;
    float totalAmount;
    time_t sellTime;  // Store the time of the sale
    struct SellRecord* next;
} SellRecord;

SellRecord* sellHistoryHead = NULL;  // Head of the sell history linked list

// Structure for sale
typedef struct Sale {
    Product* product;
    int quantity;
    struct Sale* next;
} Sale;

Sale* saleStack = NULL;

// Function prototypes for sale stack
void pushSale(Product* product, int quantity);
void undoSale();

// Global variables
Product* head = NULL;
Product* deletedStack[MAX_UNDO];
int undoTop = -1;
Product* lastSold = NULL;
int lastSoldQty = 0;
char currentUserRole[10];  // Stores the role of the logged-in user
Customer* customerHead = NULL; // Head of the customer linked list
float discountRate = 0.1;  // Default 10% discount
float premiumDiscountRate = 0.2;  // Default 20% discount for premium customers
float premiumThreshold = 500.0;  // Default threshold for premium customers

// Predefined users
User users[MAX_USERS] = {
    {"admin", "1234", "admin"},
    {"manager", "5678", "manager"}
};

// Function to validate if price and quantity are positive
int validatePriceQuantity(float price, int quantity) {
    if (price <= 0) {
        printf("Price must be a positive number.\n");
        return 0;
    }
    if (quantity <= 0) {
        printf("Quantity must be a positive number.\n");
        return 0;
    }
    return 1;
}

// Function to free customer memory
void freeCustomers() {
    Customer* current = customerHead;
    while (current) {
        Customer* temp = current;
        current = current->next;
        free(temp);
    }
    customerHead = NULL;
}

// Function to save customers to a file
void saveCustomers() {
    if (strcmp(currentUserRole, "admin") != 0) {
        printf("Access denied. Only admins can perform this action.\n");
        return;
    }

    FILE* fp = fopen("customers.dat", "wb");
    if (!fp) {
        printf("Error saving customers.\n");
        return;
    }
    Customer* current = customerHead;
    while (current) {
        fwrite(current, sizeof(Customer), 1, fp);
        current = current->next;
    }
    fclose(fp);
}

// Function to load customers from a file
void loadCustomers() {
    FILE* fp = fopen("customers.dat", "rb");
    if (!fp) {
        printf("No saved customer data found.\n");
        return;
    }
    Customer temp;
    while (fread(&temp, sizeof(Customer), 1, fp)) {
        Customer* newCustomer = (Customer*)malloc(sizeof(Customer));
        if (!newCustomer) {
            printf("Memory allocation failed.\n");
            fclose(fp);
            return;
        }
        *newCustomer = temp;
        newCustomer->next = customerHead;
        customerHead = newCustomer;
    }
    fclose(fp);
}

// Function to save sell history to a file
void saveSellHistory() {
    FILE* fp = fopen("sell_history.dat", "wb");
    if (!fp) {
        printf("Error saving sell history.\n");
        return;
    }
    SellRecord* current = sellHistoryHead;
    while (current) {
        fwrite(current, sizeof(SellRecord), 1, fp);
        current = current->next;
    }
    fclose(fp);
    printf("Sell history saved successfully.\n");
}

// Function to load sell history from a file
void loadSellHistory() {
    FILE* fp = fopen("sell_history.dat", "rb");
    if (!fp) {
        printf("No saved sell history found.\n");
        return;
    }
    SellRecord temp;
    while (fread(&temp, sizeof(SellRecord), 1, fp)) {
        SellRecord* newRecord = (SellRecord*)malloc(sizeof(SellRecord));
        if (!newRecord) {
            printf("Memory allocation failed.\n");
            fclose(fp);
            return;
        }
        *newRecord = temp;
        newRecord->next = sellHistoryHead;
        sellHistoryHead = newRecord;
    }
    fclose(fp);
    printf("Sell history loaded successfully.\n");
}

// Function to save users to a file
void saveUsers() {
    FILE* fp = fopen("users.dat", "wb");
    if (!fp) {
        printf("Error saving users.\n");
        return;
    }
    fwrite(users, sizeof(User), MAX_USERS, fp);
    fclose(fp);
    printf("Users saved successfully.\n");
}

// Function to load users from a file
void loadUsers() {
    FILE* fp = fopen("users.dat", "rb");
    if (!fp) {
        printf("No saved user data found. Using default users.\n");
        return;
    }
    fread(users, sizeof(User), MAX_USERS, fp);
    fclose(fp);
    printf("Users loaded successfully.\n");
}

// Function to show all users
void showAllUsers() {
    if (strcmp(currentUserRole, "admin") != 0) {
        printf("Access denied. Only admins can view all users.\n");
        return;
    }

    printf("\nAll Users:\n");
    printf("%-20s %-20s %-10s\n", "Username", "Password", "Role");
    printf("-----------------------------------------------\n");
    for (int i = 0; i < MAX_USERS; i++) {
        if (strlen(users[i].username) > 0) {
            printf("%-20s %-20s %-10s\n", users[i].username, users[i].password, users[i].role);
        }
    }
}

// Function to manage customer information
void customerInfoPanel() {
    int choice;
    do {
        printf("\nCustomer Information Panel\n");
        printf("1. Add Customer\n");
        printf("2. Display Customers\n");
        printf("3. Remove Customer\n");
        printf("4. Search Customer\n");  // New option
        printf("5. Manage Premium Customers\n");
        printf("6. Discount Management\n");
        printf("7. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addCustomer(); break;
            case 2: displayCustomers(); break;
            case 3: removeCustomer(); break;
            case 4: searchCustomer(); break;  // Call the searchCustomer function
            case 5: managePremiumCustomers(); break;
            case 6: discountManagement(); break;
            case 7: return;  // Exit the menu
            default: printf("Invalid choice. Try again.\n");
        }
    } while (1);
}

// Function to add a customer
void addCustomer() {
    Customer* newCustomer = (Customer*)malloc(sizeof(Customer));
    if (!newCustomer) {
        printf("Memory allocation failed.\n");
        return;
    }

    printf("Enter Customer ID: ");
    scanf("%s", newCustomer->id);

    // Check for duplicate ID
    Customer* temp = customerHead;
    while (temp) {
        if (strcmp(temp->id, newCustomer->id) == 0) {
            printf("Customer ID already exists.\n");
            free(newCustomer);
            return;
        }
        temp = temp->next;
    }

    printf("Enter Customer Name: ");
    scanf(" %[^\n]", newCustomer->name);
    printf("Enter Customer Phone: ");
    scanf("%s", newCustomer->phone);

    newCustomer->totalSpent = 0.0;
    newCustomer->premium = 0; // Default to non-premium
    newCustomer->purchaseCount = 0;
    newCustomer->next = customerHead;
    customerHead = newCustomer;

    printf("Customer added successfully.\n");
}

// Function to display customers
void displayCustomers() {
    if (!customerHead) {
        printf("No customers available.\n");
        return;
    }

    Customer* current = customerHead;
    printf("\n%-10s %-20s %-15s %-10s %-10s\n", "ID", "Name", "Phone", "Total Spent", "Premium");
    printf("-----------------------------------------------------------------\n");
    while (current) {
        printf("%-10s %-20s %-15s %-10.2f %-10s\n",
               current->id, current->name, current->phone, current->totalSpent,
               current->premium ? "Yes" : "No");
        current = current->next;
    }
}

// Function to remove a customer
void removeCustomer() {
    char id[20];
    printf("Enter Customer ID to remove: ");
    scanf("%s", id);

    Customer *current = customerHead, *prev = NULL;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            if (prev) prev->next = current->next;
            else customerHead = current->next;

            free(current);
            printf("Customer removed successfully.\n");
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("Customer not found.\n");
}

// Function to free product memory
void freeProducts() {
    Product* current = head;
    while (current) {
        Product* temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
}

// Function to save products to a file
void saveProducts() {
    FILE* fp = fopen("products.dat", "wb");
    if (!fp) {
        printf("Error saving products.\n");
        return;
    }
    Product* current = head;
    while (current) {
        fwrite(current, sizeof(Product), 1, fp);
        current = current->next;
    }
    fclose(fp);
    printf("Products saved successfully.\n");
}

// Function to manage products
void manageProductsPanel() {
    int choice;
    do {
        printf("\n\033[34mManage Products Panel\033[0m\n");
        printf("\033[33m1. Add Product\n");
        printf("2. Update Product\n");
        printf("3. Delete Product\n");
        printf("4. Undo Delete\n");
        printf("5. Display Products\n");
        printf("6. Search Product\n");
        printf("7. Search Product by Category\n");
        printf("8. Sort by Price\n");
        printf("9. Show Low Stock\n");
        printf("10. Update Stock\n");  // New option
        printf("11. Clear All Stocks\n");
        printf("12. Clear All Products\n");
        printf("13. Back to Main Menu\n");
        printf("\033[0mEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addProduct(); break;
            case 2: updateProduct(); break;
            case 3: deleteProduct(); break;
            case 4: undoDelete(); break;
            case 5: displayProducts(); break;
            case 6: searchProduct(); break;
            case 7: searchByCategory(); break;
            case 8: sortByPrice(); break;
            case 9: showLowStock(); break;
            case 10: updateStock(); break;  // Call Update Stock function
            case 11: clearAllStocks(); break;
            case 12: clearAllProducts(); break;
            case 13: return;  // Exit the manage products panel
            default: printf("\033[31mInvalid choice. Try again.\033[0m\n");  // Red text for error
        }
    } while (1);
}

// Function to add a product
void addProduct() {
    Product* newNode = (Product*)malloc(sizeof(Product));
    if (!newNode) {
        printf("Memory allocation failed.\n");
        return;
    }

    while (getchar() != '\n');  // Clear the input buffer

    printf("Enter Product ID: ");
    fgets(newNode->id, sizeof(newNode->id), stdin);
    newNode->id[strcspn(newNode->id, "\n")] = '\0';  // Remove trailing newline

    // Check for duplicate ID
    Product* temp = head;
    while (temp) {
        if (strcmp(temp->id, newNode->id) == 0) {
            printf("Product ID already exists.\n");
            free(newNode);
            return;
        }
        temp = temp->next;
    }

    printf("Enter Product Name: ");
    scanf(" %[^\n]", newNode->name);
    printf("Enter Product Category: ");
    scanf(" %[^\n]", newNode->category);
    newNode->price = getFloatInput("Enter Product Price: ");
    newNode->quantity = getIntInput("Enter Product Quantity: ");

    if (!validatePriceQuantity(newNode->price, newNode->quantity)) {
        free(newNode);
        return;
    }

    newNode->next = head;
    head = newNode;
    printf("Product added successfully.\n");
}

// Function to display products
void displayProducts() {
    if (!head) {
        printf("\033[31mNo products available.\033[0m\n");  // Red text for no products
        return;
    }

    // Header for Display Products Section
    printf("\033[34m=========================================\n");
    printf("               Product List\n");
    printf("=========================================\033[0m\n");

    // Table Header
    printf("\033[33m%-10s %-20s %-15s %-10s %-10s\033[0m\n", "ID", "Name", "Category", "Price", "Qty");
    printf("\033[34m---------------------------------------------------------------\033[0m\n");

    // Display each product
    Product* current = head;
    while (current) {
        printf("\033[32m%-10s %-20s %-15s %-10.2f %-10d\033[0m\n",
               current->id, current->name, current->category,
               current->price, current->quantity);
        current = current->next;
    }

    // Footer
    printf("\033[34m---------------------------------------------------------------\033[0m\n");
}

// Function to update a product
void updateProduct() {
    char id[20];
    printf("Enter Product ID to update: ");
    scanf("%s", id);
    Product* current = head;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("Enter new name: ");
            scanf(" %[^\n]", current->name);
            printf("Enter new category: ");
            scanf(" %[^\n]", current->category);
            current->price = getFloatInput("Enter new price: ");
            current->quantity = getIntInput("Enter Product Quantity: ");
            if (!validatePriceQuantity(current->price, current->quantity)) {
                return;
            }
            printf("Product updated.\n");
            return;
        }
        current = current->next;
    }
    printf("Product not found.\n");
}

// Function to delete a product
void deleteProduct() {
    char id[20];
    printf("Enter Product ID to delete: ");
    scanf("%s", id);
    Product *current = head, *prev = NULL;

    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("Are you sure you want to delete this product? (y/n): ");
            char confirm;
            scanf(" %c", &confirm);
            if (tolower(confirm) != 'y') {
                printf("Deletion canceled.\n");
                return;
            }

            // Push the product onto the deleted stack
            if (undoTop < MAX_UNDO - 1) {
                deletedStack[++undoTop] = current;
            } else {
                printf("\033[31mUndo stack is full. Cannot store more deleted products.\033[0m\n");
            }

            // Remove the product from the linked list
            if (prev) prev->next = current->next;
            else head = current->next;

            printf("Product deleted.\n");
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Product not found.\n");
}

// Function to push a sale onto the sale stack
void pushSale(Product* product, int quantity) {
    Sale* newSale = (Sale*)malloc(sizeof(Sale));
    newSale->product = product;
    newSale->quantity = quantity;
    newSale->next = saleStack;
    saleStack = newSale;
}

// Function to undo the last sale
void undoSale() {
    if (!saleStack) {
        printf("Nothing to undo.\n");
        return;
    }
    Sale* lastSale = saleStack;
    saleStack = saleStack->next;
    lastSale->product->quantity += lastSale->quantity;
    free(lastSale);
    printf("Last sale undone.\n");
}

// Function to undo the last deleted product
void undoDelete() {
    if (undoTop == -1) {
        printf("\033[31mNo deleted products to undo.\033[0m\n");  // Red text for error
        return;
    }

    // Restore the last deleted product
    Product* restoredProduct = deletedStack[undoTop--];
    restoredProduct->next = head;
    head = restoredProduct;

    printf("\033[32mLast deleted product restored successfully.\033[0m\n");  // Green text for success
}

// Function to sell a product
void sellProduct() {
    char id[20];
    int qty;
    float totalAmount = 0.0;  // Initialize totalAmount to 0
    float discount = 0.0;     // Initialize discount to 0
    char customerId[20], customerName[50], customerPhone[15];
    Product* selectedProducts[50];
    int selectedQuantities[50];
    int selectedCount = 0;

    // Header for Sell Product Section
    printf("\033[34m=========================================\n");
    printf("               Sell Product\n");
    printf("=========================================\033[0m\n");

    // Get customer information
    printf("\033[33mEnter Customer ID: \033[0m");
    scanf("%s", customerId);

    // Check if customer already exists
    Customer* existingCustomer = customerHead;
    while (existingCustomer) {
        if (strcmp(existingCustomer->id, customerId) == 0) {
            printf("\033[32mExisting customer found: %s (%s)\033[0m\n", existingCustomer->name, existingCustomer->phone);
            strcpy(customerName, existingCustomer->name);
            strcpy(customerPhone, existingCustomer->phone);
            break;
        }
        existingCustomer = existingCustomer->next;
    }

    // If customer does not exist, take new customer information
    if (!existingCustomer) {
        printf("\033[33mEnter Customer Name: \033[0m");
        scanf(" %[^\n]", customerName);
        printf("\033[33mEnter Customer Phone: \033[0m");
        scanf("%s", customerPhone);
    }

    // Select multiple products
    while (1) {
        printf("\033[33mEnter Product ID to sell (or type 'done' to finish): \033[0m");
        scanf("%s", id);
        if (strcmp(id, "done") == 0) break;

        Product* current = head;
        while (current) {
            if (strcmp(current->id, id) == 0) {
                printf("\033[32mAvailable quantity: %d\033[0m\n", current->quantity);
                qty = getIntInput("\033[33mEnter quantity to sell: \033[0m");
                if (qty > current->quantity) {
                    printf("\033[31mNot enough stock.\033[0m\n");
                    break;
                }

                // Add product to the selected list
                selectedProducts[selectedCount] = current;
                selectedQuantities[selectedCount] = qty;
                selectedCount++;

                // Update total amount
                totalAmount += qty * current->price;

                // Push sale onto the sale stack
                pushSale(current, qty);
                break;
            }
            current = current->next;
        }
    }

    // Calculate discount
    if (existingCustomer && existingCustomer->premium) {
        discount = totalAmount * premiumDiscountRate;  // Apply premium discount
    } else {
        // Apply regular discount for non-premium customers
        char applyDiscount;
        printf("\033[33mCustomer is not premium. Do you want to apply the regular discount (%.2f%%)? (y/n): \033[0m", discountRate * 100);
        scanf(" %c", &applyDiscount);
        if (tolower(applyDiscount) == 'y') {
            discount = totalAmount * discountRate;
        }
    }

    // Confirm sell
    printf("\n\033[34mSelected Products:\033[0m\n");
    printf("\033[33m%-10s %-20s %-10s %-10s\033[0m\n", "ID", "Name", "Qty", "Price");
    for (int i = 0; i < selectedCount; i++) {
        printf("%-10s %-20s %-10d %-10.2f\n",
               selectedProducts[i]->id, selectedProducts[i]->name,
               selectedQuantities[i], selectedProducts[i]->price);
    }
    printf("\033[33mTotal Amount: \033[32m$%.2f\033[0m\n", totalAmount);  // Display total amount
    if (discount > 0) {
        printf("\033[33mDiscount Applied: \033[31m-$%.2f\033[0m\n", discount);  // Display discount
        printf("\033[33mFinal Amount After Discount: \033[32m$%.2f\033[0m\n", totalAmount - discount);  // Display final amount
    }

    char confirm;
    printf("\033[33mConfirm sell? (y/n): \033[0m");
    scanf(" %c", &confirm);
    if (tolower(confirm) != 'y') {
        printf("\033[31mSell canceled.\033[0m\n");
        return;
    }

    // Deduct quantities and store customer history
    for (int i = 0; i < selectedCount; i++) {
        selectedProducts[i]->quantity -= selectedQuantities[i];

        // Add to sell history
        SellRecord* newRecord = (SellRecord*)malloc(sizeof(SellRecord));
        if (!newRecord) {
            printf("\033[31mMemory allocation failed for sell history.\033[0m\n");
            return;
        }
        strcpy(newRecord->customerId, customerId);
        strcpy(newRecord->customerName, customerName);
        strcpy(newRecord->productId, selectedProducts[i]->id);
        strcpy(newRecord->productName, selectedProducts[i]->name);
        newRecord->quantity = selectedQuantities[i];
        newRecord->price = selectedProducts[i]->price;
        newRecord->totalAmount = selectedQuantities[i] * selectedProducts[i]->price;
        newRecord->sellTime = time(NULL);  // Record the current time
        newRecord->next = sellHistoryHead;
        sellHistoryHead = newRecord;  // Add to the head of the sell history list
    }

    // Add or update customer history
    if (!existingCustomer) {
        Customer* newCustomer = (Customer*)malloc(sizeof(Customer));
        if (!newCustomer) {
            printf("\033[31mMemory allocation failed.\033[0m\n");
            return;
        }
        strcpy(newCustomer->id, customerId);
        strcpy(newCustomer->name, customerName);
        strcpy(newCustomer->phone, customerPhone);
        newCustomer->totalSpent = totalAmount - discount;  // Update total spent with discount
        newCustomer->purchaseCount = selectedCount;
        newCustomer->premium = (newCustomer->totalSpent > premiumThreshold) ? 1 : 0;  // Use the premium threshold
        newCustomer->next = customerHead;
        customerHead = newCustomer;
    } else {
        existingCustomer->totalSpent += (totalAmount - discount);  // Update total spent with discount
        existingCustomer->premium = (existingCustomer->totalSpent > premiumThreshold) ? 1 : 0;  // Use the premium threshold
    }

    printf("\033[32mSell completed successfully. Final Amount: $%.2f\033[0m\n", totalAmount - discount);  // Confirm final amount
}

// Function to display sell history
void showSellHistory() {
    if (!sellHistoryHead) {
        printf("No sell history available.\n");
        return;
    }

    SellRecord* current = sellHistoryHead;
    printf("\nSell History:\n");
    printf("%-10s %-20s %-20s %-10s %-10s %-20s\n", "Customer ID", "Customer Name", "Product Name", "Qty", "Price", "Time");
    while (current) {
        printf("%-10s %-20s %-20s %-10d %-10.2f %-20s",
               current->customerId, current->customerName, current->productName,
               current->quantity, current->price, ctime(&current->sellTime));
        current = current->next;
    }
}

// Function to show daily sell history
void showDailySellHistory() {
    if (!sellHistoryHead) {
        printf("No sell history available.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* today = localtime(&now);
    today->tm_hour = 0;
    today->tm_min = 0;
    today->tm_sec = 0;
    time_t startOfDay = mktime(today);

    SellRecord* current = sellHistoryHead;
    float totalSales = 0.0;

    printf("\nDaily Sell History:\n");
    printf("%-10s %-20s %-20s %-10s %-10s %-20s\n", "Customer ID", "Customer Name", "Product Name", "Qty", "Price", "Time");
    while (current) {
        if (current->sellTime >= startOfDay) {
            printf("%-10s %-20s %-20s %-10d %-10.2f %-20s",
                   current->customerId, current->customerName, current->productName,
                   current->quantity, current->price, ctime(&current->sellTime));
            totalSales += current->totalAmount;
        }
        current = current->next;
    }

    printf("\nTotal Sales for Today: $%.2f\n", totalSales);
}

// Function to show sell history for a time period
void showSellHistoryForPeriod() {
    if (!sellHistoryHead) {
        printf("No sell history available.\n");
        return;
    }

    int days;
    printf("Enter the number of days for the time period (e.g., 7 for last 7 days): ");
    scanf("%d", &days);

    time_t now = time(NULL);
    time_t startTime = now - (days * 24 * 60 * 60);  // Calculate the start time

    SellRecord* current = sellHistoryHead;
    float totalSales = 0.0;

    printf("\nSell History for the Last %d Days:\n", days);
    printf("%-10s %-20s %-20s %-10s %-10s %-20s\n", "Customer ID", "Customer Name", "Product Name", "Qty", "Price", "Time");
    while (current) {
        if (current->sellTime >= startTime) {
            printf("%-10s %-20s %-20s %-10d %-10.2f %-20s",
                   current->customerId, current->customerName, current->productName,
                   current->quantity, current->price, ctime(&current->sellTime));
            totalSales += current->totalAmount;
        }
        current = current->next;
    }

    printf("\nTotal Sales for the Last %d Days: $%.2f\n", days, totalSales);
}

// Function to get integer input with validation
int getIntInput(const char* prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            return value;
        } else {
            printf("Invalid input. Please enter a valid number.\n");
            while (getchar() != '\n');  // Clear the input buffer
        }
    }
}

// Function to get float input with validation
float getFloatInput(const char* prompt) {
    float value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &value) == 1) {
            return value;
        } else {
            printf("Invalid input. Please enter a valid number.\n");
            while (getchar() != '\n');  // Clear the input buffer
        }
    }
}

// Function to load products from a file
void loadProducts() {
    FILE* fp = fopen("products.dat", "rb");
    if (!fp) {
        printf("No saved product data found.\n");
        return;
    }
    Product temp;
    while (fread(&temp, sizeof(Product), 1, fp)) {
        Product* newProduct = (Product*)malloc(sizeof(Product));
        if (!newProduct) {
            printf("Memory allocation failed.\n");
            fclose(fp);
            return;
        }
        *newProduct = temp;
        newProduct->next = head;
        head = newProduct;
    }
    fclose(fp);
    printf("Products loaded successfully.\n");
}

// Function for user login
int login() {
    char username[20], password[20];
    int attempts = 0;

    while (attempts < 3) {
        printf("\033[34m=========================================\n");
        printf("               Login Screen\n");
        printf("=========================================\033[0m\n");
        printf("Enter username: ");
        scanf("%s", username);

        printf("Enter password: ");
        int i = 0;
        char ch;
        while (1) {
            ch = getch();  // Read a single character without echoing it
            if (ch == '\r') {  // Enter key pressed
                password[i] = '\0';
                break;
            } else if (ch == '\b') {  // Backspace key pressed
                if (i > 0) {
                    i--;
                    printf("\b \b");  // Erase the last character on the screen
                }
            } else if (i < sizeof(password) - 1) {  // Limit password length
                password[i++] = ch;
                printf("*");  // Print a '*' for each character
            }
        }
        printf("\n");

        // Check credentials
        for (int i = 0; i < MAX_USERS; i++) {
            if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
                strcpy(currentUserRole, users[i].role);  // Set the role of the logged-in user
                printf("\033[32mLogin successful! Role: %s\033[0m\n", currentUserRole);  // Green text for success
                return 1;
            }
        }

        printf("\033[31mInvalid username or password. Try again.\033[0m\n");  // Red text for error
        attempts++;
    }

    printf("\033[31mToo many failed attempts. Exiting...\033[0m\n");
    return 0;
}

// Function to search for a product
void searchProduct() {
    char id[20];
    printf("Enter Product ID to search: ");
    scanf("%s", id);

    Product* current = head;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("Product found:\n");
            printf("ID: %s, Name: %s, Category: %s, Price: %.2f, Quantity: %d\n",
                   current->id, current->name, current->category, current->price, current->quantity);
            return;
        }
        current = current->next;
    }
    printf("Product not found.\n");
}

// Function to search for products by category
void searchByCategory() {
    char category[30];
    printf("Enter Product Category to search: ");
    scanf(" %[^\n]", category);

    Product* current = head;
    int found = 0;
    printf("\nProducts in Category: %s\n", category);
    printf("%-10s %-20s %-10s %-10s\n", "ID", "Name", "Price", "Qty");
    printf("-----------------------------------------------\n");
    while (current) {
        if (strcmp(current->category, category) == 0) {
            printf("%-10s %-20s %-10.2f %-10d\n",
                   current->id, current->name, current->price, current->quantity);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No products found in this category.\n");
    }
}

// Function to sort products by price
void sortByPrice() {
    if (!head || !head->next) {
        printf("Not enough products to sort.\n");
        return;
    }

    for (Product* i = head; i != NULL; i = i->next) {
        for (Product* j = i->next; j != NULL; j = j->next) {
            if (i->price > j->price) {
                // Swap product data
                Product temp = *i;
                *i = *j;
                *j = temp;

                // Restore the next pointers
                Product* tempNext = i->next;
                i->next = j->next;
                j->next = tempNext;
            }
        }
    }
    printf("Products sorted by price.\n");
}

// Function to show low stock products
void showLowStock() {
    int threshold = 5;  // Define low stock threshold
    Product* current = head;
    int found = 0;

    printf("\nLow Stock Products (Quantity < %d):\n", threshold);
    printf("%-10s %-20s %-10s %-10s\n", "ID", "Name", "Price", "Qty");
    printf("-----------------------------------------------\n");
    while (current) {
        if (current->quantity < threshold) {
            printf("%-10s %-20s %-10.2f %-10d\n",
                   current->id, current->name, current->price, current->quantity);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No low stock products found.\n");
    }
}

// Function to clear all stocks
void clearAllStocks() {
    Product* current = head;
    while (current) {
        current->quantity = 0;
        current = current->next;
    }
    printf("All product stocks cleared.\n");
}

// Function to clear all products
void clearAllProducts() {
    Product* current = head;
    while (current) {
        Product* temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
    printf("All products cleared.\n");
}

// Function to update stock
void updateStock() {
    char id[20];
    printf("Enter Product ID to update stock: ");
    scanf("%s", id);

    Product* current = head;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("Current Stock: %d\n", current->quantity);
            current->quantity = getIntInput("Enter new stock quantity: ");
            if (current->quantity < 0) {
                printf("\033[31mStock quantity cannot be negative. Update canceled.\033[0m\n");
                return;
            }
            printf("\033[32mStock updated successfully. New Stock: %d\033[0m\n", current->quantity);
            return;
        }
        current = current->next;
    }
    printf("\033[31mProduct not found.\033[0m\n");
}

// Function to display the main menu
void menu() {
    int choice;
    do {
        printf("\033[34m");  // Set text color to blue
        printf("\n=========================================\n");
        printf("         Supermarket Management System\n");
        printf("=========================================\n");
        printf("\033[0m");  // Reset text color
        printf("\033[33m");  // Set text color to yellow
        printf("1. Admin Panel\n");
        printf("2. Customer Information\n");
        printf("3. Manage Products\n");
        printf("4. Sell Product\n");
        printf("5. Undo Sale\n");
        printf("6. Display Products\n");
        printf("7. Sell History Management\n");
        printf("8. Logout\n");
        printf("9. Save & Exit\n");
        printf("\033[0m");  // Reset text color
        printf("\033[34m=========================================\033[0m\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: adminPanel(); break;
            case 2: customerInfoPanel(); break;
            case 3: manageProductsPanel(); break;
            case 4: sellProduct(); break;
            case 5: undoSale(); break;
            case 6: displayProducts(); break;
            case 7: sellHistoryManagement(); break;
            case 8:
                printf("\033[31mLogging out...\033[0m\n");  // Red text for logging out
                return;  // Return to the login screen
            case 9:
                saveProducts();
                saveCustomers();
                saveSellHistory();
                saveUsers();  // Save users before exiting
                freeProducts();
                freeCustomers();
                printf("\033[32mChanges saved. Exiting...\033[0m\n");  // Green text for success
                exit(0);
            default: printf("\033[31mInvalid choice. Try again.\033[0m\n");  // Red text for error
        }
    } while (1);
}

// Function to reset sell history
void resetSellHistory() {
    if (!sellHistoryHead) {
        printf("No sell history to reset.\n");
        return;
    }

    SellRecord* current = sellHistoryHead;
    while (current) {
        SellRecord* temp = current;
        current = current->next;
        free(temp);
    }

    sellHistoryHead = NULL;  // Reset the head pointer
    printf("Sell history has been reset.\n");
}

// Function to manage sell history
void sellHistoryManagement() {
    int choice;
    do {
        printf("\nSell History Management\n");
        printf("1. Show Sell History\n");
        printf("2. Show Daily Sell History\n");  // New option
        printf("3. Show Sell History for a Time Period\n");  // New option
        printf("4. Reset Sell History\n");
        printf("5. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: showSellHistory(); break;
            case 2: showDailySellHistory(); break;  // Call daily sell history function
            case 3: showSellHistoryForPeriod(); break;  // Call time period sell history function
            case 4: resetSellHistory(); break;
            case 5: return;  // Exit the Sell History Management menu
            default: printf("Invalid choice. Try again.\n");
        }
    } while (1);
}

// Function to manage admin panel
void adminPanel() {
    if (strcmp(currentUserRole, "admin") != 0) {
        printf("\033[31mAccess denied. Only admins can access this panel.\033[0m\n");
        return;
    }

    int choice;
    do {
        printf("\033[34m=========================================\n");
        printf("               Admin Panel\n");
        printf("=========================================\033[0m\n");
        printf("\033[33m1. Add User\n");
        printf("2. Remove User\n");
        printf("3. Show All Users\n");
        printf("4. Back to Main Menu\n");
        printf("\033[0m=========================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addUser(); break;
            case 2: removeUser(); break;
            case 3: showAllUsers(); break;
            case 4: return;  // Exit the admin panel
            default: printf("\033[31mInvalid choice. Try again.\033[0m\n");
        }
    } while (1);
}

// Function to add a user
void addUser() {
    if (strcmp(currentUserRole, "admin") != 0) {
        printf("Access denied. Only admins can add users.\n");
        return;
    }

    char username[20], password[20], role[10];
    printf("Enter new username: ");
    scanf("%s", username);

    // Check if the username already exists
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("Username already exists. Try a different username.\n");
            return;
        }
    }

    printf("Enter new password: ");
    scanf("%s", password);
    printf("Enter role (e.g., admin, manager): ");
    scanf("%s", role);

    // Add the new user to the first available slot
    for (int i = 0; i < MAX_USERS; i++) {
        if (strlen(users[i].username) == 0) {
            strcpy(users[i].username, username);
            strcpy(users[i].password, password);
            strcpy(users[i].role, role);
            printf("User added successfully.\n");
            return;
        }
    }

    printf("User limit reached. Cannot add more users.\n");
}

// Function to remove a user
void removeUser() {
    if (strcmp(currentUserRole, "admin") != 0) {
        printf("Access denied. Only admins can remove users.\n");
        return;
    }

    char username[20];
    printf("Enter username to remove: ");
    scanf("%s", username);

    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Clear the user data
            users[i].username[0] = '\0';
            users[i].password[0] = '\0';
            users[i].role[0] = '\0';
            printf("User '%s' removed successfully.\n", username);
            return;
        }
    }

    printf("User not found.\n");
}

// Function to change user credentials
void changeUserCredentials() {
    printf("Change Username and Password functionality not implemented yet.\n");
}

// Function to search for a customer
void searchCustomer() {
    char id[20];
    printf("Enter Customer ID to search: ");
    scanf("%s", id);

    Customer* current = customerHead;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("\nCustomer Details:\n");
            printf("ID: %s\n", current->id);
            printf("Name: %s\n", current->name);
            printf("Phone: %s\n", current->phone);
            printf("Total Spent: %.2f\n", current->totalSpent);
            printf("Premium Status: %s\n", current->premium ? "Yes" : "No");
            return;
        }
        current = current->next;
    }
    printf("Customer not found.\n");
}

// Function to manage premium customers
void managePremiumCustomers() {
    int choice;
    do {
        printf("\nManage Premium Customers\n");
        printf("1. Add Premium Customer\n");
        printf("2. Remove Premium Customer\n");
        printf("3. Display Premium Customers\n");
        printf("4. Search Customer\n");  // New option
        printf("5. Edit Minimum Amount for Premium Customers\n");
        printf("6. Back to Customer Information Panel\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addPremiumCustomer(); break;
            case 2: removePremiumCustomer(); break;
            case 3: displayPremiumCustomers(); break;
            case 4: searchCustomer(); break;  // Call the new searchCustomer function
            case 5: editPremiumThreshold(); break;
            case 6: return;  // Exit the menu
            default: printf("Invalid choice. Try again.\n");
        }
    } while (1);
}

// Function to add a premium customer
void addPremiumCustomer() {
    char id[20];
    printf("Enter Customer ID to make premium: ");
    scanf("%s", id);

    Customer* current = customerHead;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            current->premium = 1;  // Mark as premium
            printf("Customer %s is now a premium customer.\n", current->name);
            return;
        }
        current = current->next;
    }
    printf("Customer not found.\n");
}

// Function to remove a premium customer
void removePremiumCustomer() {
    char id[20];
    printf("Enter Customer ID to remove from premium: ");
    scanf("%s", id);

    Customer* current = customerHead;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            current->premium = 0;  // Remove premium status
            printf("Customer %s is no longer a premium customer.\n", current->name);
            return;
        }
        current = current->next;
    }
    printf("Customer not found.\n");
}

// Function to display premium customers
void displayPremiumCustomers() {
    Customer* current = customerHead;
    int found = 0;

    printf("\nPremium Customers:\n");
    printf("%-10s %-20s %-15s %-10s\n", "ID", "Name", "Phone", "Total Spent");
    printf("---------------------------------------------------\n");
    while (current) {
        if (current->premium) {
            printf("%-10s %-20s %-15s %-10.2f\n",
                   current->id, current->name, current->phone, current->totalSpent);
            found = 1;
        }
        current = current->next;
    }
    if (!found) {
        printf("No premium customers found.\n");
    }
}

// Function to edit premium buy amount
void editPremiumBuyAmount() {
    char id[20];
    printf("Enter Customer ID to edit buy amount: ");
    scanf("%s", id);

    Customer* current = customerHead;
    while (current) {
        if (strcmp(current->id, id) == 0) {
            printf("Current Total Spent: %.2f\n", current->totalSpent);
            current->totalSpent = getFloatInput("Enter new total spent amount: ");
            current->premium = (current->totalSpent > premiumThreshold) ? 1 : 0;  // Update premium status
            printf("Buy amount updated. Premium status: %s\n", current->premium ? "Yes" : "No");
            return;
        }
        current = current->next;
    }
    printf("Customer not found.\n");
}

// Function to edit premium threshold
void editPremiumThreshold() {
    printf("Current Premium Threshold: %.2f\n", premiumThreshold);
    premiumThreshold = getFloatInput("Enter new premium threshold amount: ");
    printf("Premium threshold updated to %.2f.\n", premiumThreshold);

    // Update the premium status of all customers
    Customer* current = customerHead;
    while (current) {
        current->premium = (current->totalSpent > premiumThreshold) ? 1 : 0;
        current = current->next;
    }
    printf("Premium status of customers updated based on the new threshold.\n");
}

// Function to manage discount rates
void discountManagement() {
    int choice;
    do {
        printf("\nDiscount Management\n");
        printf("1. Edit Regular Discount Percentage\n");
        printf("2. Edit Premium Discount Percentage\n");
        printf("3. Back to Customer Information Panel\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                discountRate = getFloatInput("Enter new regular discount percentage (e.g., 0.1 for 10%): ");
                printf("Regular discount updated to %.2f%%.\n", discountRate * 100);
                break;
            case 2:
                premiumDiscountRate = getFloatInput("Enter new premium discount percentage (e.g., 0.2 for 20%): ");
                printf("Premium discount updated to %.2f%%.\n", premiumDiscountRate * 100);
                break;
            case 3: return;  // Exit the menu
            default: printf("Invalid choice. Try again.\n");
        }
    } while (1);
}

int main() {
    printf("\033[34m=========================================\n");
    printf("       Welcome to Supermarket System\n");
    printf("=========================================\033[0m\n");

    loadUsers();       // Load users at the start
    loadProducts();
    loadCustomers();
    loadSellHistory(); // Load sell history at the start

    while (1) {
        if (!login()) {
            printf("\033[31mLogin failed. Try again.\033[0m\n");
            continue;  // Retry login
        }
        menu();
    }

    saveUsers();       // Save users before exiting
    saveProducts();
    saveCustomers();
    saveSellHistory(); // Save sell history before exiting
    freeProducts();
    freeCustomers();
    return 0;
}
