#include <iostream>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>

#define WINVER 0x0502//#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "vengheader.h"

using namespace std;

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE); // For use of SetConsoleTextAttribute() - Sets console to default. (aka sets canvas to command prompt)
COORD CursorPosition;
CONSOLE_CURSOR_INFO cursorInfo;

void MoveCursor(int, int); // Move cursor to (x, y)
char CheckKeyPressed(); // Check if a key is being pressed and return which key.
int CheckNumPressed(); // Check if a key is being pressed and return which key.
bool CheckYesNo(char, char); // Check if user pushes [char / char]. Example: (Y / N)
int Menu(int, int, int, int, bool); // Menu (x1, y1, x2, y2, center[true/false])returns 0 = exit; 1 = play; 2 = continue; 
void PrintGUI(int, int, int, int, int);
void PrintInfo(string type, Player& pl, Inventory& inv);
void PrintInfoDay(Player& pl);
void ClearConsole(int, int, int, int);
void Teleport(Player&, Map&, Dialogue&);
void npcInteraction(string, Player&, Inventory&, Map&, Dialogue&);
void AddlineSlow(string, bool, Dialogue&);
HWND consoleWin = GetConsoleWindow();


int const windowWidth = 29;  // Map Display Width
int const windowHeight = 23; // Map Display Height
string const OUTOFBOUNDS = "We don't know what's over there...";
string const HITAWALL = "Try not to walk into walls in the future...";
string const HITWATER = "You don't know how to swim!";
string const HITTREE = "Trees don't make the greatest of friends... try the town nearby.";
string const HITPATH = "You take a bold step out onto the path.";
string const HITRUIN = "You walk into the ruins of your old town...";
string const TELEPORTING = "Now entering ";
char const WATER = '~';
char const WALL = 'W';
char const TELEPORT = '@';
char const TREE = 'T';
char const PATH = 'P';
char const RUIN = 'R';
char const BUSH = 'B';
int const WORLDFIGHTCHANCE = 400; // Fight chance in normal world. 1/400 steps will initiate a fight.
int WILDFIGHTCHANCE = 80; // Fight chance in wilderness. 1/80 steps will initiate a fight. NOT A CONST BECAUSE IT WILL CHANGE WITH DEPTH INTO WILDERNESS.
int const DIALOGUEDELAYTIME = 500; // Delay time between each dialogue paragraph or sentence.
int const KEYISPRESSED = 0x8000; // Bitwise operator number to return true or false if ([certain key] & KEYISPRESSED) = true.

int main() {
	string gameStatus = "MENU";
	string userInput;
	bool inBattle = false;
	char keyPressed;

	int infoWidth = 15;
	int infoHeight = windowHeight;
	int dialogueHeight = 10;
	int hudGap = 3;

	char tempChar;
	int tempInt;
	bool tempBool;

	Map world;

	SetConsoleTextAttribute(console, 15);

	RECT r;
	GetWindowRect(consoleWin, &r); //stores the console's current dimensions
	MoveWindow(consoleWin, r.left, r.top, (windowWidth + infoWidth + hudGap) * 16, (4 + windowHeight + dialogueHeight) * 18, TRUE);

	GetConsoleCursorInfo(console, &cursorInfo); // Get info then...
	cursorInfo.bVisible = false;                // Set cursor blinking off.
	SetConsoleCursorInfo(console, &cursorInfo); // ^^^^^^^^^^^^^^^^^^^^^^^

	SetConsoleTitle("Final Project  ::  Vengeance");

	int menuInput;
	string tempStr;

	MoveCursor(0, 0);

	PrintGUI(windowWidth, windowHeight, infoWidth, dialogueHeight, hudGap);
	Dialogue dialogue(3, 28, windowWidth + hudGap + infoWidth, 39);
	Player player;
	Inventory inventory;
	inventory.Add("Bulky Stick", "Weapon", 2, 2);
	inventory.Add("Jewel", "Valuable", 25);
	inventory.Add("Food", "Food", 2, 0, 0, 10);
	dialogue.Addline("Welcome to Vengeance.");
	Sleep(100);
	dialogue.Addline("Push [P] to play");
	dialogue.Addline("Push [C] to continue (coming soon!)");
	dialogue.Addline("Push [Q] to quit");
	srand(time(0)); // RANDOM SEED

	while (true) {
		MoveCursor(0, 0);

		while (gameStatus == "MENU") {
			MoveCursor(0, 0);
			menuInput = Menu(3, 4, (windowWidth - 1), (windowHeight - 1), true); // Draw menu as well as recieve input.
			
			switch (menuInput) {
			case -1:
				break;
			case 0: // Quit
				return 0;
			case 1:
				gameStatus = "Running";
				break;
			case 2:
				break;
			default:
				break;
			}
		}


		dialogue.AddlineSlow("You used to live in a small remote village, with small cottages littered sparingly");
		dialogue.AddlineSlow("between neighbors. ");
		Sleep(DIALOGUEDELAYTIME);
		dialogue.AddlineSlow("It wasn't a great life, but it was a peaceful and fullfilling life, and one that you");
		dialogue.AddlineSlow("enjoyed.");
		Sleep(DIALOGUEDELAYTIME);
		dialogue.AddlineSlow("You wished you could keep this life, but fate had other plans in store for you, and grimones.");
		Sleep(DIALOGUEDELAYTIME);
		dialogue.AddlineSlow("One sunny peaceful day, a demon lord appeared.");
		Sleep(DIALOGUEDELAYTIME);
		dialogue.AddlineSlow("Your life was over in the blink of an eye, and everything you ever knew, everything you ever ");
		dialogue.AddlineSlow("loved, was gone, never to be seen again.", false);
		Sleep(DIALOGUEDELAYTIME);
		dialogue.Addline("");
		dialogue.AddlineSlow("You are the lone survivor.");
		Sleep(DIALOGUEDELAYTIME);
		dialogue.Addline("");
		dialogue.AddlineSlow("This is your quest for Vengeance.");

		Sleep(DIALOGUEDELAYTIME);

		dialogue.Addline("");
		dialogue.Addline("(*) Push [H] for help.");

		PrintInfo("Stats", player, inventory);
		string infoSet = "Statistics";

		while (gameStatus == "Running") {
			Sleep(10); // -------------------------------------------------------------------------------

			player.AddDay();
			if (infoSet == "Statistics") {
				PrintInfoDay(player);
			}
			int px = player.GetX();
			int py = player.GetY();

			// Print Top GUI
			player.PrintHealth();
			player.PrintExperience();
			player.PrintLevel();
			player.PrintGold();

			world.PrintMap(px, py, 3, 4, windowWidth, windowHeight);

			int random = 100;
			int cMap = world.GetMap();

			if (cMap == 2) { // World
				random = rand() % WORLDFIGHTCHANCE;
			}
			else if (cMap == 4) { // Wilderness
				random = rand() % WILDFIGHTCHANCE;
			}
			if (random >= 0 && random <= 3) {
				inBattle = true;
			}
			if (inBattle) {
				PrintInfo("Combat", player, inventory);
				Enemy enemy;

				if (cMap == 2) { // World
					if (random < 2) {
						enemy.Init("bandit", (1 + (player.GetLevel() / 2)));
					}
					else if (random == 2) {
						enemy.Init("cult-member", (1 + (player.GetLevel() / 4)));
					}
					else if (random == 3) {
						enemy.Init("animal", (1 + (player.GetLevel() / 2)));
					}
				}
				else if (cMap == 4) { // Wilderness
					enemy.Init("animal", player.GetLevel() + ((50 - py) / 5)); // Animal level scales with depth into wilderness and player's level.
				}

				if (enemy.GetName() == "bandit") {
					dialogue.Addline("A dark figure slides out from behind a nearby tree.");
					dialogue.Addline("You're being robbed by a bandit!");
					dialogue.Addline("He's wielding a cutlass. Be careful! He looks dangerous.");
				}
				else if (enemy.GetName() == "cult-member") {
					dialogue.AddlineSlow("You here grunting noises in the distance...they don't sound human...", true, 10);
					dialogue.AddlineSlow("Something is getting closer, get ready for a fight!", true, 10);
					dialogue.AddlineSlow("A man comes running out of the bushes covered in bloody scratches and half torn rags.", true, 10);
					dialogue.AddlineSlow("Sharp nails and teeth...and so little coverage...seems to be a close combat fighter.", true, 10);
					dialogue.AddlineSlow("He must be a crazy cult-member from the nearby cave. Keep your distance!", true, 10);
					dialogue.AddlineSlow("...mostly for the nudity.", true, 10);
				}
				else if (enemy.GetName() == "animal") {
					dialogue.Addline("You've been attacked by a wild " + enemy.GetName() + "!");
				}
				else {
					dialogue.Addline("Error - enemy-related: Likely undefined enemy.");
				}
				while (inBattle == true) { // Bandit, animals, cult-member.
					Sleep(125);
					 //  ----------------CHANGE: Add inititaly FIGHT OR FLIGHT [F]light or [R]un. Higher chances of escape. If fails, initiate fight anyway. "[enemy] is too fast for you, you've been initiated in a fight!"
					keyPressed = '~'; 
					while (!(keyPressed == 'A' || keyPressed == 'P' || keyPressed == 'F')) {
						keyPressed = CheckKeyPressed();
					}

					if (keyPressed == 'A') { // Attack
						enemy.Attack(player, dialogue);
						if (enemy.GetHealth() <= 0) {
							int dev = rand() % player.GetLevel(); // Deviation increments with level.
							inBattle = false;

							int expGain = (enemy.GetDiff() * 2) + dev; // EXPERIENCE gain = (enemy level * 2) + (random deviation based off player level)
							int moneyGain = (enemy.GetDiff()) + dev; // MONEY gain = (enemy level) + (random deviation based off player level)
							dialogue.Addline("You killed " + enemy.GetName());
							player.AddExp(expGain);
							player.AddGold(moneyGain); 
							//cout << "+" << moneyGain << "Gold" << "\n+" << expGain  << "Experience" << endl; MAKE THIS PRINT OUT IN A BUFFER THAT ADDS TO DIALOGUE!!
							continue; // inBattle is now false, check that and skip the rest of the while loop.
						}
					}
					else if (keyPressed == 'P') { // Potion
						tempInt = inventory.Find("Health Potion");
						if (tempInt > 0) {
							inventory.Delete("Health Potion");
							dialogue.Addline("You drink a health potion. (+25 health)");
							player.AddHealth(25);
						}
						else {
							tempInt = inventory.Find("Food");
							if (tempInt > 0) {
								inventory.Delete("Food");
								dialogue.Addline("You eat food. (+20 health)");
								player.AddHealth(20);
							}
						}
					}
					else if (keyPressed == 'F') { // Flee
						random = rand() % 4; // 25% chance
						if (random != 1) {
							inBattle = false;
							dialogue.Addline("You run away!");
						}
						else {
							dialogue.Addline("You attempt to run away...");
						}
					}
					Sleep(DIALOGUEDELAYTIME / 2); // Because it's annoyingly fast.
					if (inBattle == true) {
						enemy.TakeTurn(player, dialogue); // Make sure player hasn't run away beforehand enemy hits again.
					}
				}
				PrintInfo(infoSet, player, inventory);
				player.PrintExperience();
				player.PrintGold();
			}


			keyPressed = '~';
			while (keyPressed == 'A' || keyPressed == 'P' || keyPressed == 'F' || keyPressed == '~') {
				keyPressed = '~';
				keyPressed = CheckKeyPressed();
			}
			if (keyPressed == '^') {
				if (py - 1 > 0) {
					if (world.GetAtPos(px, py - 1) == 'C') {
						npcInteraction("cook", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py - 1) == 'D') {
						npcInteraction("doctor", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py - 1) == 'B') {
						npcInteraction("blacksmith", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py - 1) == '~') {
						dialogue.Addline(HITWATER);
					}
					else if (world.GetAtPos(px, py - 1) == TREE) {
						dialogue.Addline(HITTREE);
					}
					else if (world.GetAtPos(px, py - 1) != WALL) {
						player.Move(0, -1); // Move up
											//dialogue.Addline("Moved north 1 square. ");
						if (world.GetAtPos(px, py - 1) == TELEPORT && world.GetAtPos(px, py) != TELEPORT) {
							Teleport(player, world, dialogue);
						}
						else if (world.GetAtPos(px, py - 1) == PATH && world.GetAtPos(px, py) != PATH) {
							dialogue.Addline(HITPATH);
						}
						else if (world.GetAtPos(px, py - 1) == RUIN && world.GetAtPos(px, py) != RUIN) {
							dialogue.Addline(HITRUIN);
						}
					}
					else {
						dialogue.Addline(HITAWALL);
					}
				}
				else {
					dialogue.Addline(OUTOFBOUNDS);
				}
			}
			else if (keyPressed == '\\') {
				if (py + 1 < world.GetHeight()) {
					if (world.GetAtPos(px, py + 1) == 'C') {
						npcInteraction("cook", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py + 1) == 'D') {
						npcInteraction("doctor", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py + 1) == 'B') {
						npcInteraction("blacksmith", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px, py + 1) == '~') {
						dialogue.Addline(HITWATER);
					}
					else if (world.GetAtPos(px, py + 1) == TREE) {
						dialogue.Addline(HITTREE);
					}
					else if (world.GetAtPos(px, py + 1) != WALL) {
						player.Move(0, 1); // Move up
										   //dialogue.Addline("Moved south 1 square. ");
						if (world.GetAtPos(px, py + 1) == TELEPORT && world.GetAtPos(px, py) != TELEPORT) {
							Teleport(player, world, dialogue);
						}
						else if (world.GetAtPos(px, py + 1) == PATH && world.GetAtPos(px, py) != PATH) {
							dialogue.Addline(HITPATH);
						}
						else if (world.GetAtPos(px, py + 1) == RUIN && world.GetAtPos(px, py) != RUIN) {
							dialogue.Addline(HITRUIN);
						}
					}
					else {
						dialogue.Addline(HITAWALL);
					}
				}
				else {
					dialogue.Addline(OUTOFBOUNDS);
				}
			}
			else if (keyPressed == '>') {
				if (px + 1 < world.GetWidth()) {
					if (world.GetAtPos(px + 1, py) == 'C') {
						npcInteraction("cook", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px + 1, py) == 'D') {
						npcInteraction("doctor", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px + 1, py) == 'B') {
						npcInteraction("blacksmith", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px + 1, py) == '~') {
						dialogue.Addline(HITWATER);
					}
					else if (world.GetAtPos(px + 1, py) == TREE) {
						dialogue.Addline(HITTREE);
					}
					else if (world.GetAtPos(px + 1, py) != WALL) {
						player.Move(1, 0); // Move up
										   //dialogue.Addline("Moved east 1 square. ");
						if (world.GetAtPos(px + 1, py) == TELEPORT && world.GetAtPos(px, py) != TELEPORT) {
							Teleport(player, world, dialogue);
						}
						else if (world.GetAtPos(px + 1, py) == PATH && world.GetAtPos(px, py) != PATH) {
							dialogue.Addline(HITPATH);
						}
						else if (world.GetAtPos(px + 1, py) == RUIN && world.GetAtPos(px, py) != RUIN) {
							dialogue.Addline(HITRUIN);
						}
					}
					else {
						dialogue.Addline(HITAWALL);
					}
				}
				else {
					dialogue.Addline(OUTOFBOUNDS);
				}
			}
			else if (keyPressed == '<') {
				if (px - 1 > 0) {
					if (world.GetAtPos(px - 1, py) == 'C') {
						npcInteraction("cook", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px - 1, py) == 'D') {
						npcInteraction("doctor", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px - 1, py) == 'B') {
						npcInteraction("blacksmith", player, inventory, world, dialogue);
						PrintInfo(infoSet, player, inventory);
					}
					else if (world.GetAtPos(px - 1, py) == '~') {
						dialogue.Addline(HITWATER);
					}
					else if (world.GetAtPos(px - 1, py) == TREE) {
						dialogue.Addline(HITTREE);
					}
					else if (world.GetAtPos(px - 1, py) != WALL) {
						player.Move(-1, 0); // Move left
											//dialogue.Addline("Moved west 1 square. ");
						if (world.GetAtPos(px - 1, py) == TELEPORT && world.GetAtPos(px, py) != TELEPORT) {
							Teleport(player, world, dialogue);
						}
						else if (world.GetAtPos(px - 1, py) == PATH && world.GetAtPos(px, py) != PATH) {
							dialogue.Addline(HITPATH);
						}
						else if (world.GetAtPos(px - 1, py) == RUIN && world.GetAtPos(px, py) != RUIN) {
							dialogue.Addline(HITRUIN);
						}
					}
					else {
						dialogue.Addline(HITAWALL);
					}
				}
				else {
					dialogue.Addline(OUTOFBOUNDS);
					//tempStr = world.GetAtPos(px-1, py);
					//dialogue.Addline(tempStr, 15);
				}
			}
			else if (keyPressed == 'E') { // Examine
				dialogue.Addline("What would you like to examine? [0-9]");
				tempInt = CheckNumPressed();
				inventory.Examine(dialogue, world, px, py, tempInt);
			}
			else if (keyPressed == 'I') { // Inventory
				if (inventory.GetSize() <= 0) {
					dialogue.Addline("You're not holding anything right now.");
				}
				PrintInfo("Inventory", player, inventory);
				infoSet = "Inventory";
			}
			else if (keyPressed == 'W') { // Wield Item
				PrintInfo("Inventory", player, inventory);
				if (inventory.GetSize() <= 0) {
					dialogue.Addline("You're not holding any weapons.");
				}
				else {
					dialogue.Addline("What would you like to wield? [1-9]");
					tempInt = -1;
					while (!(tempInt >= 0 && tempInt < inventory.GetCap())) {
						tempInt = CheckNumPressed();
					}
					if (inventory.GetType(tempInt) == "Weapon") {
						player.AddAtkGear(-(inventory.GetWeaponPtr()->GetAtk()));
						player.AddDefGear(-(inventory.GetArmorPtr()->GetDef()));
						inventory.Wield(tempInt);
						player.AddAtkGear(inventory.GetWeaponPtr()->GetAtk());
						player.AddDefGear(inventory.GetArmorPtr()->GetDef());
						dialogue.Addline("You wield " + inventory.GetWeapon());
						PrintInfo(infoSet, player, inventory); // Reprint
					}
					else if (inventory.GetType(tempInt) == "Armor") {
						player.AddAtkGear(-(inventory.GetWeaponPtr()->GetAtk()));
						player.AddDefGear(-(inventory.GetArmorPtr()->GetDef()));
						inventory.Wield(tempInt);
						player.AddAtkGear(inventory.GetWeaponPtr()->GetAtk());
						player.AddDefGear(inventory.GetArmorPtr()->GetDef());
						dialogue.Addline("You put on " + inventory.GetArmor());
						PrintInfo(infoSet, player, inventory); // Reprint
					}
					else {
						dialogue.Addline("You can't wield that!");
					}
				}
				PrintInfo(infoSet, player, inventory);
			}
			else if (keyPressed == 'D') { // Drop Item
				if (inventory.GetSize() <= 0) {
					dialogue.Addline("You're not holding anything right now.");
				}
				else {
					dialogue.Addline("What would you like to drop? [1-9]");
					tempInt = -1;
					while (!(tempInt >= 0 && tempInt < inventory.GetCap())) {
						tempInt = CheckNumPressed();
					}
					dialogue.Addline("Are you sure you want to drop this? You will not get this back. [Y / N]");
					Sleep(DIALOGUEDELAYTIME);
					tempBool = CheckYesNo('Y', 'N'); // Check if player pushes Y or N.
					tempStr = inventory.GetName(tempInt);
					if (tempBool) {
						tempBool = inventory.DeleteAtPos(tempInt);
						if (tempBool) {
							dialogue.Addline("Dropped " + tempStr);
							PrintInfo(infoSet, player, inventory);
						}
					}
					else {
						dialogue.Addline("(*) You realize you're not a very decisive person. (*)");
					}
				}
			}
			else if (keyPressed == 'S') { // Statistics
				dialogue.Addline("Now showing statistics.");
				PrintInfo("Statistics", player, inventory);
				infoSet = "Statistics";
			}
			else if (keyPressed == 'H') { // Help
				dialogue.Addline("List of commands: ");
				dialogue.Addline("[H]elp");
				dialogue.Addline("[I]nventory");
				dialogue.Addline("[S]tatistics");
				dialogue.Addline("[W]ield or [W]ear");
				dialogue.Addline("[E]xamine");
				dialogue.Addline("[D]rop Item");
				dialogue.Addline("[Q]uit");
			}
			else if (keyPressed == 'Q') { // Quit
				dialogue.Addline("The game may not be saved. Are you sure you want to quit? ", true);
				Sleep(25);
				dialogue.Addline("Enter:  [Y / N] ");
				keyPressed = false;
				while (keyPressed == false) {
					if (GetKeyState('Y') & KEYISPRESSED) {
						return 0;
					}
					else if (GetKeyState('N') & KEYISPRESSED) {
						keyPressed = true; // Do nothing.
						dialogue.Addline("OK. ");
					}
				}
			}
			MoveCursor(3, 1);
			px = player.GetX();
			py = player.GetY();
		}
	}
}

// ----------------------END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-------
// ----END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-------
// ----------------------END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-----------------------------END-MAIN-------

void MoveCursor(int x, int y)
{
	CursorPosition.X = x;
	CursorPosition.Y = y;
	SetConsoleCursorPosition(console, CursorPosition);
}

void ClearMapWindow(int x1, int y1, int x2, int y2) {
	for (int i = y1; i < y2; ++i) {
		MoveCursor(x1, i);
		cout << setfill(' ') << setw(x2 - x1) << " ";
	}

}

bool CheckYesNo(char key1, char key2) {
	int tempInt = -1;

	while (tempInt == -1) {
		if (GetKeyState(key1) & KEYISPRESSED) {
			tempInt = 1;
		}
		else if (GetKeyState(key2) & KEYISPRESSED) {
			tempInt = 0;
		}
	}

	bool tempBool = false;
	if (tempInt == 1) {
		tempBool = true;
	}
	else if (tempInt == 0) {
		tempBool = false;
	}

	return tempBool;
}

char CheckKeyPressed() {
	char tempChar = '~';
	bool keyPressed = false;
	while (keyPressed == false) {
		if (GetKeyState('I') & KEYISPRESSED) {
			tempChar = 'I';
		}
		else if (GetKeyState('S') & KEYISPRESSED) {
			tempChar = 'S';
		}
		else if (GetKeyState(VK_UP) & KEYISPRESSED) {
			tempChar = '^'; // North
		}
		else if (GetKeyState(VK_DOWN) & KEYISPRESSED) {
			tempChar = '\\';
		}
		else if (GetKeyState(VK_LEFT) & KEYISPRESSED) {
			tempChar = '<';
		}
		else if (GetKeyState(VK_RIGHT) & KEYISPRESSED) {
			tempChar = '>';
		}
		else if (GetKeyState('A') & KEYISPRESSED) {
			tempChar = 'A';
		}
		else if (GetKeyState('D') & KEYISPRESSED) {
			tempChar = 'D';
		}
		else if (GetKeyState('E') & KEYISPRESSED) {
			tempChar = 'E';
		}
		else if (GetKeyState('F') & KEYISPRESSED) {
			tempChar = 'F';
		}
		else if (GetKeyState('Q') & KEYISPRESSED) {
			tempChar = 'Q';
		}
		else if (GetKeyState('W') & KEYISPRESSED) {
			tempChar = 'W';
		}
		else if (GetKeyState('P') & KEYISPRESSED) {
			tempChar = 'P';
		}
		else if (GetKeyState('C') & KEYISPRESSED) {
			tempChar = 'C';
		}
		else if (GetKeyState('H') & KEYISPRESSED) {
			tempChar = 'H';
		}
		if (tempChar != '~') {
			Sleep(50);
			return tempChar;
			//break;
		}
	}
}

int CheckNumPressed() {
	int tempInt = '~';
	bool keyPressed = false;
	while (keyPressed == false) {
		if (GetKeyState('1') & KEYISPRESSED) {
			return 0;
		}
		else if (GetKeyState('2') & KEYISPRESSED) {
			tempInt = 1;
		}
		else if (GetKeyState('3') & KEYISPRESSED) {
			tempInt = 2;
		}
		else if (GetKeyState('4') & KEYISPRESSED) {
			tempInt = 3;
		}
		else if (GetKeyState('5') & KEYISPRESSED) {
			tempInt = 4;
		}
		else if (GetKeyState('6') & KEYISPRESSED) {
			tempInt = 5;
		}
		else if (GetKeyState('7') & KEYISPRESSED) {
			tempInt = 6;
		}
		else if (GetKeyState('8') & KEYISPRESSED) {
			tempInt = 7;
		}
		else if (GetKeyState('9') & KEYISPRESSED) {
			tempInt = 8;
		}
		else if (GetKeyState('0') & KEYISPRESSED) {
			tempInt = -1;
		}
		if (tempInt != '~') {
			Sleep(50);
			return tempInt;
		}
	}
}

int Menu(int x1, int y1, int x2, int y2, bool center) {
	char keyPressed = '~';
	int centerX = (x2 - x1) + 4;
	int centerY = (y2 - y1) + 4 - 2; // shift up 2 spaces for looks

	if (center) {
		MoveCursor((centerX - 12), (centerY - 12));
		cout << "_________[MENU]_________";

		MoveCursor((centerX - 12), (centerY - 10));
		cout << " [-]      Play      [-]";
		MoveCursor((centerX - 12), (centerY - 8));
		cout << " [-]    Continue    [-]";
		MoveCursor((centerX - 12), (centerY)-6);
		cout << " [-]      Quit      [-]";

		for (int i = (centerY - 4); i < centerY; ++i) {
			MoveCursor((centerX - 10), i);
			cout << "-" << right << setw((centerX / 2) + 5) << "-";
		}
		MoveCursor(centerX - 1, centerY);
		cout << "--";

		while (keyPressed == '~') { // Get input.
			keyPressed = CheckKeyPressed();
		}

		if (keyPressed == 'P') {
			return 1;
		}
		else if (keyPressed == 'C') {
			return 2;
		}
		else if (keyPressed == 'Q') {
			bool tempBool = false;
			while (true) {
				MoveCursor((centerX - 7), (centerY - 4));
				cout << "Art thou sure?";
				MoveCursor((centerX - 5), (centerY - 2));
				cout << "[ Y || N ]";
				tempBool = CheckYesNo('Y', 'N');
				if (tempBool == true) {
					break;
				}
				else { // if tempBool == false, meaning player pushed N
					tempBool = false;
					ClearMapWindow((centerX - 8), (centerY - 4), (centerX + 8), (centerY)); // Clean up text if User says [N]o.
					return -1;
					break;
				}
			}

			if (tempBool == true) {
				return 0;
			}
		}
		else {
			return (-1);
		}
		keyPressed = '~';
	}
}

void Teleport(Player& playerP, Map& mapP, Dialogue& dialogueP) {
	string printLast = "Teleporting...";
	if (playerP.GetX() > mapP.GetWidth() - 5) { // Right-hand side of map
		if (mapP.GetMap() == 1) { // Intro map - World
			mapP.SetMap(2);
			playerP.Place(1, 13);
		}
		else if (mapP.GetMap() == 2) { // World map - Extro
			if (playerP.BossUnlocked() == true) {
				mapP.SetMap(5);
				playerP.Place(1, 13);
			}
			else {
				printLast = "You wouldn't dare venture in there without a torch.";
			}
		}
	}
	else if (playerP.GetX() < 5) { // left-hand side of map
		if (mapP.GetMap() == 2) { // World Map - Intro
			mapP.SetMap(1);
			playerP.Place(mapP.GetWidth() - 1, 19);
		}
		else if (mapP.GetMap() == 5) { // Extro map - World
			mapP.SetMap(2);
			playerP.Place(mapP.GetWidth() - 1, 13);
		}
	}
	else if (playerP.GetY() < 5) { // top side of map
		if (mapP.GetMap() == 2) { // World map - Wilderness
			mapP.SetMap(4);
			playerP.Place(20, mapP.GetHeight() - 1);
		}
		else if (mapP.GetMap() == 3) { // Town map - World
			mapP.SetMap(2);
			playerP.Place(27, mapP.GetHeight() - 1);
		}
	}
	else if (playerP.GetY() > mapP.GetHeight() - 5) { // bottom side of map
		if (mapP.GetMap() == 2) { // World Map - Town
			mapP.SetMap(3);
			playerP.Place(21, 1);
		}
		else if (mapP.GetMap() == 4) { // Wilderness map - World
			mapP.SetMap(2);
			playerP.Place(27, 1);
		}
	}
	dialogueP.Addline(printLast);
	if (printLast == "Teleporting...") {
		dialogueP.Addline(TELEPORTING + mapP.GetMapName());
	}
}

void npcInteraction(string name, Player& player, Inventory& inventory, Map& world, Dialogue& dialogue) {
	NPC npc(name);
	bool done = false;
	while (done == false) {
		char keyPressed = '~';
		while (keyPressed == '~') {
			keyPressed = CheckKeyPressed();
		}

		// WILL KEEP LOOPING ARE RETURNING DIFFERENT LETTERS THE 
		// PLAYER PUSHED UNTIL YOU HAVE DONE = TRUE

		if (name == "cook") {
			int progress = player.GetCook("progress");

			if (progress == 0) {
				dialogue.AddlineSlow("The smell of fresh bread immediately hits your nose as you enter the bakery. ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Inside, stands a seemingly aged man... maybe around his fifties? ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("He's attending to the display of different pastries before he looks up to see you walk");
				dialogue.AddlineSlow("in.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Cook: Hello there! Haven't seen you around before, you new here?");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("You nod your head and give a small \"yep\" in response.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Cook: In that case, pleasure to make your acquaintance! I am the village's cook and");
				dialogue.AddlineSlow("baker, ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("and I don't mean to boast, but  I sell the tastiest, sweetest food you'll find intown!", false);
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Cook: Have a look see if you'd like to buy anything!");
				Sleep(DIALOGUEDELAYTIME);
				player.AddCook("progress", 1);
			}
			else {
				dialogue.AddlineSlow("Cook: Have a look see if you'd like to buy anything!");
				Sleep(DIALOGUEDELAYTIME * 1.5);
			}
			dialogue.AddlineSlow("Buy some food for 5 gold apiece? [Y / N]");
			bool tempBool = CheckYesNo('Y', 'N');
			if (tempBool) {
				if (player.GetGold() >= 5) {
					dialogue.AddlineSlow("You buy some food for 5 gold pieces.");
					player.AddGold(-5);
					player.PrintGold();
					inventory.Add("Food", "Food", 4, 0, 0, 10);
					done = true;
				}
				else {
					dialogue.AddlineSlow("You can't afford that.");
					done = true;
				}
			}
			else {
				dialogue.AddlineSlow("OK.");
				done = true;
			}
		}
		else if (name == "doctor") {
			int progress = player.GetDoc("progress");
			//player.SetDoc(0);
			if (progress == 0) {
				dialogue.AddlineSlow("As you step inside, you are greeted with a man that seems to be within his early");
				dialogue.AddlineSlow("thirties with a smile plastered accross his face.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("In one hand he holds an Erlenmeyer flask with red liquid inside and in the other a small");
				dialogue.AddlineSlow("stick.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("that has been just recently stirring said flask.", false);
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("He points to you with his right hand containing the stirring stick and begins to speak");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("with an upbeat voice.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Doctor: You look like a new visitor to our humble village! I welcome you.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Doctor: I am the local doctor in this village and I specialize in treating all our sick.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Buy a health potion for 25 gold apiece? [Y / N]");
				Sleep(DIALOGUEDELAYTIME * 1.5);
				player.AddDoc("progress", 1);
			}
			else {
				dialogue.AddlineSlow("The doc is still at work with his sciencey things and potion mixing. As you approach him,");
				dialogue.AddlineSlow("you greet him with a smile.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Doctor: Welcome back, adventurer! Will you be needing some rejuvenation potions for your");
				dialogue.AddlineSlow("journey?");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Buy a health potion for 25 gold apiece? [Y / N]");
				Sleep(DIALOGUEDELAYTIME);
			}
			bool tempBool = CheckYesNo('Y', 'N');
			if (tempBool) {
				if (player.GetGold() >= 25) {
					dialogue.AddlineSlow("You buy a health potion for 25 gold pieces.");
					player.AddGold(-25);
					player.PrintGold();
					inventory.Add("Health Potion", "Potion", 4, 0, 0, 25);
					done = true;
				}
				else {
					dialogue.AddlineSlow("You can't afford that.");
					done = true;
				}
			}
			else {
				dialogue.AddlineSlow("OK.");
				done = true;
			}
		}
		else if (name == "blacksmith") {
			int progress = player.GetBlacksmith("progress");

			if (progress == 0) {
				dialogue.AddlineSlow("You step into a small separated building that didn't match the other resident homes.");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("As you enter the doorless building, you hear the clank of hammer on steel. ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("A large man with biceps the size of your head is focused on smithing something... a");
				dialogue.AddlineSlow("sword? ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("It seems that he has just started, so it's hard to discern what it is that he's");
				dialogue.AddlineSlow("creating. ");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("He hears your footsteps as you enter the building");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("BlackSmithy: Aye, mate, ya look lost. You new in town?");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("(*) You nod your head in confirmation. (*)");
				Sleep(DIALOGUEDELAYTIME);
				dialogue.AddlineSlow("Blacksmithy: Well I'm this here village's blacksmith, th' one stop shop for all 'yer");
				dialogue.AddlineSlow("offensive and defensive needs. If ya need anything smithed, I've got th' best quality");
				dialogue.AddlineSlow("steel you'll find.");
				Sleep(DIALOGUEDELAYTIME);
				player.AddBlacksmith("progress", 1);
			}
			else {
				dialogue.AddlineSlow("The blacksmith is still hammering away at a weapon when he notices you.");
			}
			dialogue.AddlineSlow("Blacksmithy: you lookin' for an upgrade? ");
			dialogue.Addline("[Y / N]", false);
			bool tempBool = CheckYesNo('Y', 'N');
			if (tempBool == true) {
				dialogue.AddlineSlow("You give a small smile and nod as you admire his handy work. ");
				dialogue.Addline("[W] for weapons.");
				dialogue.Addline("[A] for armor.");
				bool tempBool = CheckYesNo('W', 'A');
				if (tempBool == true) {
					dialogue.AddlineSlow("Buy a sword for 45 gold pieces? ");
					dialogue.Addline("[Y / N]", false);
					tempBool = CheckYesNo('Y', 'N');
					if (tempBool) {
						if (player.GetGold() >= 45) {
							dialogue.Addline("You buy a sword for 45 gold pieces.");
							player.AddGold(-45);
							player.PrintGold();
							inventory.Add("Sword", "Weapon", 20, 4, 1, 0);
							done = true;
						}
						else {
							//string tempStr = static_cast<string>(45 - player.GetGold());
							//dialogue.Addline("Blacksmithy: Sorry mate, you're " + tempStr + " gold pieces short.");
							dialogue.Addline("Blacksmithy: Sorry mate, you're a bit short in gold for such a piece.");
							done = false;
						}
					}
					else { // Pushes no, doesn't want to buy an item.
						dialogue.Addline("(*) You reject his offer (*)");
						dialogue.AddlineSlow("You hear the blacksmithy mutter under his breath \"Your loss...\" before he regains his");
						dialogue.AddlineSlow("demeanor and says with a big smile \"Is there anything else I can do for you?");
						done = false;
					}
				}
				else {
					dialogue.Addline("Buy chainmail for 50 gold pieces? [Y / N]");
					tempBool = false;
					tempBool = CheckYesNo('Y', 'N');
					if (tempBool) {
						if (player.GetGold() >= 50) {
							dialogue.Addline("You buy chainmail for 50 gold pieces.");
							player.AddGold(-50);
							player.PrintGold();
							inventory.Add("Chainmail", "Armor", 25, 0, 3, 0);
							done = true;
						}
						else {
							dialogue.Addline("Blacksmithy: Sorry mate, you're a bit short in gold for such a piece.");
							done = false;
						}
					}
					else { // Doesn't want to buy chainmail. ADD OPTION TO VIEW INVENTORY AGAIN.
						dialogue.Addline("(*) You reject his offer (*)");
						dialogue.AddlineSlow("You hear the blacksmithy mutter under his breath \"Your loss...\" before he regains his");
						dialogue.AddlineSlow("demeanor and says with a big smile \"Is there anything else I can do for you?");
						done = false;
					}
				}
			} // Not interested in viewing inventory. Perhaps launch dialogue about the cave?
			else {
				dialogue.Addline("Blacksmithy: 'right then. I wish you the best of luck in yer' travels, mate.");
				done = true;
			}
		}
	}
}

void PrintInfo(string type, Player& pl, Inventory& inv) {

	for (int posy = 5; posy < 27; ++posy) {
		MoveCursor(62, posy);
		cout << setfill(' ') << setw(30) << "";
	}

	if (type == "Stats" || type == "Statistics") {
		MoveCursor(63, 6);
		cout << setw(16) << right << setfill(' ') << "Stats" << setw(13) << " ";
		MoveCursor(63, 8);
		cout << " -  Exp to level up: " << pl.ExpToLevel();
		MoveCursor(63, 10);
		cout << " -  Attack: " << pl.GetAtkTotal();
		MoveCursor(63, 12);
		cout << " -  Defense: " << pl.GetDefTotal();
		MoveCursor(63, 14);
		cout << " -  Wielded: " << inv.GetWeapon();
		MoveCursor(63, 16);
		cout << " -  Wearing: " << inv.GetArmor();
		MoveCursor(63, 18);
		cout << " -  Name: " << pl.GetName();
		MoveCursor(63, 20);
		cout << " -  Age: " << pl.GetAge();
		MoveCursor(63, 22);
		cout << " -  Day: " << pl.GetDay();
		MoveCursor(63, 24);
		cout << " -  Gender: " << pl.GetGender();

	}
	else if (type == "Inventory") {
		MoveCursor(63, 6);
		cout << setw(19) << right << setfill(' ') << "Inventory";
		for (int i = 0; i < inv.GetCap(); ++i) {
			MoveCursor(63, 8 + (i * 2));
			cout << " [" << i + 1 << "]  " << inv.GetName(i);
		}
	}
	else if (type == "Combat") {
		MoveCursor(63, 6);
		cout << setw(18) << right << setfill(' ') << "Combat";
		MoveCursor(63, 8);
		cout << " -  Wielded: " << inv.GetWeapon();
		MoveCursor(63, 10);
		cout << " -  Wearing: " << inv.GetArmor();
		MoveCursor(63, 12);
		cout << " [A]  Attack";
		MoveCursor(63, 14);
		cout << " [P]  Potion";
		MoveCursor(63, 16);
		cout << " [F]  Flee";
	}
}

void PrintInfoDay(Player& pl) {
	MoveCursor(63, 22);
	cout << " -  Day: " << pl.GetDay() << "  ";
}

void PrintGUI(int width, int height, int width2, int height2, int gap) {
	MoveCursor(0, 0);
	cout << "  " << setfill('-') << setw(((width + width2) * 2) + gap) << " " << endl;
	cout << " |" << setfill(' ') << setw(((width + width2) * 2) + gap) << " |" << endl;
	cout << " |" << setfill(' ') << setw(((width + width2) * 2) + gap) << " |" << endl;
	cout << "  " << setfill('-') << setw(((width + width2) * 2) + gap) << " " << setfill(' ') << endl;

	for (int yy = 0; yy < height; ++yy) {

		cout << " | " << setw((width + 1) * 2) << " | "
			<< setw((width2) * 2) << " |" << endl; // Print walls / Info!
	}

	cout << "  " << setfill('-') << setw((width + width2) * 2 + gap) << " " << setfill(' ') << endl;

	for (int yy = 0; yy < height2; ++yy) {
		cout << " |" << setw(((width + width2) * 2) + gap)
			<< right << "|" << endl; // Print walls / dialogue!
	}
	cout << "  " << setfill('-') << setw((width + width2) * 2 + gap) << " " << setfill(' ') << endl;
	MoveCursor(3, 1);
	cout << "   ____________[Health]________[Experience]_________[Level]________[Gold]____________   ";
	MoveCursor(62, 4);
	cout << setw(21) << setfill('_') << "[Information]" << setw(9) << "";
	cout << setfill(' ');
}

