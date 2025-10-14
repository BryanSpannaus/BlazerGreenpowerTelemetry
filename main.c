#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

void clear_input_buffer(void);
void monitor(void);
void mainMenu(void);
void saveData(void);
void loadData(void);
void modeSelect(void);
void setSerial(void);
void dataView(void);
void deviceCFG(void);
int main(void);

bool deviceConnected = false;
float versionNum = 0.2;
char exitC = 'z'; //pretty useful
char SERIAL_PORT[32]; 
int baudRate;

void clear_input_buffer(void) {
    while (getchar() != '\n');
}

void monitor (void) {

    int menu;
    menu = -1; //so menu doesnt repeat cycle and get locked

    system("clear");
    printf("\n\n\n\nData Monitor\n\n"); 
    printf("Device Connection State: ");
    if (deviceConnected) { //check if device is connected
        printf("TRUE\n\n");
    } else {
        printf("FALSE\n\n");
    }
    printf("[1] Connect Device \t [3] Test\n\n");
    printf("[2] View Data \t [4] Return to Main Menu\n\n"); //print options menu

    scanf("%i", &menu);//scan for menu item

    if (menu != -1)
    {

    switch(menu) //which submenu do I enter
    {
    case 1:

        break;

    case 2:
        dataView();
        break;

    case 3:
        if (deviceConnected)
        {
            deviceConnected = false;
        } else {
            deviceConnected = true;
        }
        monitor();
        break;

    case 4:
        mainMenu();
        break;
    
    default:
        printf("\n Please Select a menu item");
        monitor();
        break;
    }
}

}

void setSerial(void) {
    system("clear");
    printf("\n\n\n Input Serial Port Should Look Like /dev/ttyACM0 or /dev/ttyUSB0\n\n");
    clear_input_buffer();
    while (scanf("%s", SERIAL_PORT) == 1) //look for input
    break;
    system("clear"); //clear the screen
    printf("\nThe Serial Port Input Was: %s Press enter to continue.\n", SERIAL_PORT);
    clear_input_buffer();
    while (scanf("c") != '\n')
    break;
}

void dataView(void) {
    system("clear");

}

void deviceCFG (void) {
   int menu;

    menu = -1; //so menu doesnt repeat cycle and get locked

    system("clear");
    printf("\n\n\n\nDevice Configuration \n\n"); 
    printf("Select a item, or return to main menu. \n\n");
    printf("[1] Select Device Port and Baud Rate \t [4] Test GPS\n\n");
    printf("[2] Send Coordinates \t\t \t [5] Run Diagnostic \n\n"); //print options menu
    printf("[3] Test LoRa \t\t\t\t [6] Return to Main Menu\n\n");


    scanf("%i", &menu);//scan for menu item

    if (menu != -1)
    {
    switch(menu) //which submenu do I enter
    {
    case 1: //set baud rate and port
       setSerial();
        break;

    case 2: //upload gps points

        break;

    case 3: //initiate LoRa test

        break;

    case 4: //initiate GPS test
    

        break;

    case 5: //Runs Diagnostic test of systems
        system("clear"); //Clears Screen
        exitC = 'z'; //Set to non-y/n char
        printf("\n\nYou will run a diagnostic system test immediately, this will cease all datalogging.\n \n Type either N/n to return, or Y/n to confirm. \n \n");

        while (getchar() != EOF) {
        scanf("%c", &exitC);
        
        if (exitC == 'n' || exitC == 'N' ) {
            deviceCFG();
        } else if (exitC == 'Y' || exitC == 'y') {
            printf("running diagnostic");
        } else {
            printf("Invalid Input");
            deviceCFG();
        }
    }
        break;
    
    case 6:
       mainMenu();
        break;

    default:
        printf("\n Please Select a menu item");
        deviceCFG();
    break;
    }
}

}

void modeSelect (void) {


}

void saveData (void) {


}

void loadData (void) {


}

void mainMenu (void) {

    int menu;
    menu = -1; //so menu doesnt repeat cycle and get locked

    system("clear");
    printf("\n\n\n\nTelemetry System %.1f \n\n", versionNum); //update ver. number
    printf("Main menu, please select an option. \n\n");
    printf("[1] Monitor Data \t [4] Save Files\n\n");
    printf("[2] Mode Select \t [5] Load Files\n\n"); //print options menu
    printf("[3] Device Configuration [6] Quit\n\n");


    scanf("%i", &menu);//scan for menu item

    if (menu != -1)
    {
    switch(menu) //which submenu do I enter
    {
    case 1:
        monitor();
        break;

    case 2:
        modeSelect();
        break;

    case 3:
        deviceCFG();
        break;

    case 4:
        saveData();
        break;

    case 5:
        loadData();
        break;
    
    case 6:
        system("clear"); //Clears Screen
        exitC = 'z'; //Set to non-y/n char
        printf("\n\nYou will exit the program, all unsaved changes will be lost\n \n Type either N/n to return, or press Ctrl+C to quit. \n \n");

        while (getchar() != EOF) {
        scanf("%c", &exitC);
        
        if (exitC == 'n' || exitC == 'N' ) {
            mainMenu();
        } else {
            printf("Invalid Input");
            mainMenu();
        }
    }
        break;

    default:
        printf("\n Please Select a menu item");
        mainMenu();
    break;
    }
}
}

int main (void) {
    mainMenu(); //Bring up start menu
    

    return (0);
}
