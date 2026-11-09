/**
  * REALBOT2 : Numpad menu
  *
  * A server-side numeric-keypad menu for the listen-server host.
  * No client DLL or client-side menu files are required: the keys are
  * bound to "realbot2_menu ..." commands in realbot2_keys.cfg.
  **/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <extdll.h>
#include <dllapi.h>
#include <meta_api.h>
#include <entity_state.h>

#include "bot.h"
#include "bot_func.h"
#include "game.h"
#include "NodeMachine.h"
#include "menu.h"

extern cBot bots[32];
extern cGame Game;
extern cNodeMachine NodeMachine;

extern edict_t *pHostEdict;
extern bool draw_nodes;
extern bool draw_connodes;
extern bool autoskill;
extern bool internet_play;
extern int max_bots;
extern int num_bots;
extern int kick_amount_bots;
extern int kick_bots_team;
extern int counterstrike;
extern float bot_check_time;
extern char *rb_version_nr;

#define MENU_PAGE_CLOSED    0
#define MENU_PAGE_MAIN      1
#define MENU_PAGE_SETTINGS  2
#define MENU_PAGE_TEAM      3
#define MENU_PAGE_STATUS    4
#define MENU_PAGE_TOOLS     5
#define MENU_PAGE_MAX       5

static int menu_page[33];
static char menu_notice[33][192];

static int Menu_Index(edict_t *pEntity) {
    if (pEntity == NULL) return -1;
    int index = ENTINDEX(pEntity);
    if (index < 1 || index > 32) return -1;
    return index;
}

static bool Menu_IsAuthorized(edict_t *pEntity) {
    // The numpad menu can add/remove bots, so only the local listen-server
    // host may use it.  Dedicated servers use the normal console commands.
    if (pEntity == NULL) return false;
    if (IS_DEDICATED_SERVER()) return false;
    return (pEntity == pHostEdict);
}

static void Menu_Notice(edict_t *pEntity, const char *fmt, ...) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(menu_notice[index], fmt, argptr);
    va_end(argptr);
}

static void Menu_CountBots(int *tCount, int *ctCount, int *total) {
    *tCount = 0;
    *ctCount = 0;
    *total = 0;

    for (int i = 0; i < 32; i++) {
        if (!bots[i].bIsUsed) continue;

        (*total)++;
        if (bots[i].iTeam == 1)
            (*tCount)++;
        else if (bots[i].iTeam == 2)
            (*ctCount)++;
    }
}

static int Menu_CountHumans(void) {
    int count = 0;

    for (int i = 1; i <= gpGlobals->maxClients; i++) {
        edict_t *pPlayer = INDEXENT(i);
        if ((pPlayer == NULL) || pPlayer->free)
            continue;

        if (UTIL_GetBotPointer(pPlayer) != NULL)
            continue;

        if (pPlayer->v.flags & FL_THIRDPARTYBOT)
            continue;

        if (pPlayer->v.flags & FL_FAKECLIENT)
            continue;

        count++;
    }

    return count;
}

static int Menu_TeamCounts(int *tPlayers, int *ctPlayers) {
    int t = 0, ct = 0;

    // Count human players by their CS team value (1 = T, 2 = CT).
    for (int i = 1; i <= gpGlobals->maxClients; i++) {
        edict_t *pPlayer = INDEXENT(i);
        if ((pPlayer == NULL) || pPlayer->free)
            continue;

        if (UTIL_GetBotPointer(pPlayer) != NULL)
            continue;

        if (pPlayer->v.team == 1)
            t++;
        else if (pPlayer->v.team == 2)
            ct++;
    }

    // Add the bots.
    for (int i = 0; i < 32; i++) {
        if (!bots[i].bIsUsed) continue;

        if (bots[i].iTeam == 1)
            t++;
        else if (bots[i].iTeam == 2)
            ct++;
    }

    if (tPlayers) *tPlayers = t;
    if (ctPlayers) *ctPlayers = ct;

    return t + ct;
}

static const char *Menu_MaxBotsText(void) {
    static char text[16];
    if (max_bots < 0)
        return "unlimited";
    sprintf(text, "%d", max_bots);
    return text;
}

static int Menu_BalancedTeam(void) {
    int t = 0, ct = 0;
    Menu_TeamCounts(&t, &ct);
    return (ct <= t) ? 2 : 1;
}

static bool Menu_AddBot(int team, int skill) {
    char teamArg[8];
    char skillArg[8];

    sprintf(teamArg, "%d", team);
    if (skill == -2) {
        skillArg[0] = 0;         // use the default skill
    } else if (skill >= 0 && skill <= 10) {
        sprintf(skillArg, "%d", skill);
    } else {
        strcpy(skillArg, "-1");  // random skill
    }

    int result = Game.createBot(NULL, teamArg, skillArg, NULL, NULL);
    if (result == GAME_MSG_SUCCESS) {
        bot_check_time = gpGlobals->time + 5.0;
        return true;
    }

    return false;
}

static int Menu_FillServer(void) {
    int added = 0;
    int maxClients = gpGlobals->maxClients;
    int maxBots = (max_bots >= 0) ? max_bots : 32;

    if (maxClients > 32) maxClients = 32;
    if (maxClients < 1) maxClients = 1;
    if (maxBots > 32) maxBots = 32;

    for (int i = 0; i < 32; i++) {
        int t = 0, ct = 0, totalBots = 0;
        Menu_CountBots(&t, &ct, &totalBots);

        if (totalBots >= maxBots)
            break;

        int humans = Menu_CountHumans();
        if (humans + totalBots >= maxClients)
            break;

        int team = Menu_BalancedTeam();
        if (!Menu_AddBot(team, -2))
            break;

        added++;
    }

    return added;
}

static void Menu_KickBots(int amount, int team) {
    kick_amount_bots = amount;
    kick_bots_team = team;
}

static void Menu_PrintStatus(edict_t *pEntity) {
    int t = 0, ct = 0, totalBots = 0;
    int tPlayers = 0, ctPlayers = 0;

    Menu_CountBots(&t, &ct, &totalBots);
    Menu_TeamCounts(&tPlayers, &ctPlayers);

    Menu_Notice(pEntity,
                "Bots: %d (T %d / CT %d)   Humans: %d\n"
                "Default skill: %s   Max bots: %s\n"
                "Auto-skill: %s   Internet mode: %s\n"
                "CS mode: 1.6",
                totalBots, t, ct, Menu_CountHumans(),
                (Game.iDefaultBotSkill < 0) ? "random" :
                (Game.iDefaultBotSkill == 10 ? "easy (10)" :
                 (Game.iDefaultBotSkill == 5 ? "normal (5)" :
                  (Game.iDefaultBotSkill == 2 ? "hard (2)" :
                   (Game.iDefaultBotSkill == 0 ? "godlike (0)" : "custom")))),
                Menu_MaxBotsText(),
                autoskill ? "ON" : "OFF",
                internet_play ? "ON" : "OFF");
}

static void Menu_PrintHelp(edict_t *pEntity) {
    CLIENT_PRINTF(pEntity, print_console,
                  "REALBOT2 console commands:\n"
                  "  realbot2 add [team] [skill] [model] [name]\n"
                  "  realbot2 remove [amount] [team]\n"
                  "  realbot2 killall\n"
                  "  realbot2 skill [-1..10]\n"
                  "  realbot2 max [amount]\n"
                  "  realbot2 autoskill [0/1]\n"
                  "  realbot2 internet [0/1]\n"
                  "  realbot2 server players [amount]\n"
                  "  realbot2 nodes draw|connections|save|load\n"
                  "  realbot2 debug ...\n"
                  "Menu: bind KP_INS realbot2_menu (see realbot2_keys.cfg)\n");
}

static void Menu_PrintKeys(edict_t *pEntity) {
    CLIENT_PRINTF(pEntity, print_console,
                  "REALBOT2 numpad menu keys:\n"
                  "  KP_INS        open / close menu\n"
                  "  KP_END ... KP_PGUP   select item 1..9\n"
                  "  KP_PLUS       next page\n"
                  "  KP_MINUS      previous page\n"
                  "  KP_DEL        close menu\n"
                  "  KP_ENTER      open main page\n"
                  "Bind them with: exec realbot2_keys.cfg\n");
}

static void Menu_Render(edict_t *pEntity) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    char body[768];
    char output[1024];
    const char *notice = menu_notice[index];
    int page = menu_page[index];

    if (page == MENU_PAGE_SETTINGS) {
        sprintf(body,
                "===== REALBOT2 v%s =====\n"
                "NUMPAD MENU [SETTINGS 2/5]\n"
                "1  Easy skill        (10)\n"
                "2  Normal skill      (5)\n"
                "3  Hard skill        (2)\n"
                "4  Godlike skill     (0)\n"
                "5  Random skill      (-1)\n"
                "6  Auto-skill        : %s\n"
                "7  Internet mode     : %s\n"
                "8  Max bots          : %s\n"
                "9  Back to main\n"
                "KP_INS toggle   KP_PLUS/KP_MINUS page   KP_DEL close",
                rb_version_nr,
                autoskill ? "ON" : "OFF",
                internet_play ? "ON" : "OFF",
                Menu_MaxBotsText());
    } else if (page == MENU_PAGE_TEAM) {
        sprintf(body,
                "===== REALBOT2 v%s =====\n"
                "NUMPAD MENU [TEAM 3/5]\n"
                "1  Add 1 T bot\n"
                "2  Add 1 CT bot\n"
                "3  Add 1 T bot (hard)\n"
                "4  Add 1 CT bot (hard)\n"
                "5  Remove 1 T bot\n"
                "6  Remove 1 CT bot\n"
                "7  Kick all T bots\n"
                "8  Kick all CT bots\n"
                "9  Back to main\n"
                "KP_INS toggle   KP_PLUS/KP_MINUS page   KP_DEL close",
                rb_version_nr);
    } else if (page == MENU_PAGE_STATUS) {
        int t = 0, ct = 0, totalBots = 0;
        Menu_CountBots(&t, &ct, &totalBots);

        sprintf(body,
                "===== REALBOT2 v%s =====\n"
                "NUMPAD MENU [STATUS 4/5]\n"
                "Bots: %d  (T %d / CT %d)\n"
                "Humans: %d\n"
                "Default skill: %s\n"
                "Max bots: %s   Auto-skill: %s\n"
                "Internet mode: %s   CS mode: 1.6\n"
                "1  Refresh status\n"
                "8  Tools >\n"
                "9  Back to main\n"
                "KP_INS toggle   KP_PLUS/KP_MINUS page   KP_DEL close",
                rb_version_nr,
                totalBots, t, ct,
                Menu_CountHumans(),
                (Game.iDefaultBotSkill < 0) ? "random" :
                (Game.iDefaultBotSkill == 10 ? "easy (10)" :
                 (Game.iDefaultBotSkill == 5 ? "normal (5)" :
                  (Game.iDefaultBotSkill == 2 ? "hard (2)" :
                   (Game.iDefaultBotSkill == 0 ? "godlike (0)" : "custom")))),
                Menu_MaxBotsText(),
                autoskill ? "ON" : "OFF",
                internet_play ? "ON" : "OFF");
    } else if (page == MENU_PAGE_TOOLS) {
        sprintf(body,
                "===== REALBOT2 v%s =====\n"
                "NUMPAD MENU [TOOLS 5/5]\n"
                "1  Print command help (console)\n"
                "2  Draw nodes         : %s\n"
                "3  Draw connections   : %s\n"
                "4  Save nodes\n"
                "5  Load nodes\n"
                "6  Version information\n"
                "7  Numpad key help\n"
                "8  Status >\n"
                "9  Back to main\n"
                "KP_INS toggle   KP_PLUS/KP_MINUS page   KP_DEL close",
                rb_version_nr,
                draw_nodes ? "ON" : "OFF",
                draw_connodes ? "ON" : "OFF");
    } else {
        sprintf(body,
                "===== REALBOT2 v%s =====\n"
                "NUMPAD MENU [MAIN 1/5]\n"
                "1  Quick add bot (balanced)\n"
                "2  Add T bot\n"
                "3  Add CT bot\n"
                "4  Fill server with bots\n"
                "5  Remove 1 bot\n"
                "6  Remove all bots\n"
                "7  Settings >\n"
                "8  Team tools >\n"
                "9  Status & tools >\n"
                "KP_INS toggle   KP_PLUS/KP_MINUS page   KP_DEL close",
                rb_version_nr);
    }

    if ((notice != NULL) && (notice[0] != 0))
        sprintf(output, "%s\n%s", notice, body);
    else
        strcpy(output, body);

    CLIENT_PRINTF(pEntity, print_center, output);
}

static void Menu_ChangePage(edict_t *pEntity, int page) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    if (page < MENU_PAGE_MAIN) page = MENU_PAGE_MAX;
    if (page > MENU_PAGE_MAX) page = MENU_PAGE_MAIN;

    menu_page[index] = page;
    menu_notice[index][0] = 0;
    Menu_Render(pEntity);
}

static void Menu_Select(edict_t *pEntity, int item) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    if (menu_page[index] == MENU_PAGE_CLOSED) {
        menu_page[index] = MENU_PAGE_MAIN;
        menu_notice[index][0] = 0;
    }

    int page = menu_page[index];

    if (page == MENU_PAGE_MAIN) {
        switch (item) {
            case 1:
                if (Menu_AddBot(Menu_BalancedTeam(), -2))
                    Menu_Notice(pEntity, "Added a bot to the smaller team.");
                else
                    Menu_Notice(pEntity, "Could not add a bot (server full).");
                break;
            case 2:
                if (Menu_AddBot(1, -2)) Menu_Notice(pEntity, "Added a terrorist bot.");
                else Menu_Notice(pEntity, "Could not add a T bot.");
                break;
            case 3:
                if (Menu_AddBot(2, -2)) Menu_Notice(pEntity, "Added a counter-terrorist bot.");
                else Menu_Notice(pEntity, "Could not add a CT bot.");
                break;
            case 4: {
                int added = Menu_FillServer();
                if (added > 0)
                    Menu_Notice(pEntity, "Added %d bot(s).", added);
                else
                    Menu_Notice(pEntity, "No bot added (server is full).");
                break;
            }
            case 5:
                Menu_KickBots(1, 0);
                Menu_Notice(pEntity, "Removing 1 bot...");
                break;
            case 6:
                Menu_KickBots(31, 0);
                Menu_Notice(pEntity, "Removing all bots...");
                break;
            case 7:
                Menu_ChangePage(pEntity, MENU_PAGE_SETTINGS);
                return;
            case 8:
                Menu_ChangePage(pEntity, MENU_PAGE_TEAM);
                return;
            case 9:
                Menu_ChangePage(pEntity, MENU_PAGE_STATUS);
                return;
        }
    } else if (page == MENU_PAGE_SETTINGS) {
        switch (item) {
            case 1:
                Game.iDefaultBotSkill = 10;
                Menu_Notice(pEntity, "Default skill set to easy (10).");
                break;
            case 2:
                Game.iDefaultBotSkill = 5;
                Menu_Notice(pEntity, "Default skill set to normal (5).");
                break;
            case 3:
                Game.iDefaultBotSkill = 2;
                Menu_Notice(pEntity, "Default skill set to hard (2).");
                break;
            case 4:
                Game.iDefaultBotSkill = 0;
                Menu_Notice(pEntity, "Default skill set to godlike (0).");
                break;
            case 5:
                Game.iDefaultBotSkill = -1;
                Menu_Notice(pEntity, "Default skill set to random (-1).");
                break;
            case 6:
                autoskill = !autoskill;
                Menu_Notice(pEntity, "Auto-skill is now %s.", autoskill ? "ON" : "OFF");
                break;
            case 7:
                internet_play = !internet_play;
                Menu_Notice(pEntity, "Internet simulation is now %s.", internet_play ? "ON" : "OFF");
                break;
            case 8: {
                static const int caps[] = {-1, 8, 12, 16, 24, 31};
                int current = 0;
                for (int i = 0; i < 6; i++) {
                    if (caps[i] == max_bots) {
                        current = i;
                        break;
                    }
                }
                max_bots = caps[(current + 1) % 6];
                Menu_Notice(pEntity, "Max bots set to %s.", Menu_MaxBotsText());
                break;
            }
            case 9:
                Menu_ChangePage(pEntity, MENU_PAGE_MAIN);
                return;
        }
    } else if (page == MENU_PAGE_TEAM) {
        switch (item) {
            case 1:
                if (Menu_AddBot(1, -2)) Menu_Notice(pEntity, "Added a T bot.");
                else Menu_Notice(pEntity, "Could not add a T bot.");
                break;
            case 2:
                if (Menu_AddBot(2, -2)) Menu_Notice(pEntity, "Added a CT bot.");
                else Menu_Notice(pEntity, "Could not add a CT bot.");
                break;
            case 3:
                if (Menu_AddBot(1, 2)) Menu_Notice(pEntity, "Added a hard T bot.");
                else Menu_Notice(pEntity, "Could not add a T bot.");
                break;
            case 4:
                if (Menu_AddBot(2, 2)) Menu_Notice(pEntity, "Added a hard CT bot.");
                else Menu_Notice(pEntity, "Could not add a CT bot.");
                break;
            case 5:
                Menu_KickBots(1, 1);
                Menu_Notice(pEntity, "Removing 1 terrorist bot...");
                break;
            case 6:
                Menu_KickBots(1, 2);
                Menu_Notice(pEntity, "Removing 1 counter-terrorist bot...");
                break;
            case 7:
                Menu_KickBots(31, 1);
                Menu_Notice(pEntity, "Kicking all terrorist bots...");
                break;
            case 8:
                Menu_KickBots(31, 2);
                Menu_Notice(pEntity, "Kicking all counter-terrorist bots...");
                break;
            case 9:
                Menu_ChangePage(pEntity, MENU_PAGE_MAIN);
                return;
        }
    } else if (page == MENU_PAGE_STATUS) {
        switch (item) {
            case 1:
                Menu_PrintStatus(pEntity);
                break;
            case 8:
                Menu_ChangePage(pEntity, MENU_PAGE_TOOLS);
                return;
            case 9:
                Menu_ChangePage(pEntity, MENU_PAGE_MAIN);
                return;
        }
    } else if (page == MENU_PAGE_TOOLS) {
        switch (item) {
            case 1:
                Menu_PrintHelp(pEntity);
                Menu_Notice(pEntity, "Command help printed to console.");
                break;
            case 2:
                draw_nodes = !draw_nodes;
                Menu_Notice(pEntity, "Node drawing is now %s.", draw_nodes ? "ON" : "OFF");
                break;
            case 3:
                draw_connodes = !draw_connodes;
                Menu_Notice(pEntity, "Connection drawing is now %s.", draw_connodes ? "ON" : "OFF");
                break;
            case 4:
                NodeMachine.save();
                Menu_Notice(pEntity, "Nodes saved.");
                break;
            case 5:
                NodeMachine.load();
                Menu_Notice(pEntity, "Nodes loaded.");
                break;
            case 6:
                Menu_Notice(pEntity, "REALBOT2 v%s - based on RealBot by Stefan Hendriks.", rb_version_nr);
                break;
            case 7:
                Menu_PrintKeys(pEntity);
                Menu_Notice(pEntity, "Numpad key help printed to console.");
                break;
            case 8:
                Menu_ChangePage(pEntity, MENU_PAGE_STATUS);
                return;
            case 9:
                Menu_ChangePage(pEntity, MENU_PAGE_MAIN);
                return;
        }
    }

    Menu_Render(pEntity);
}

void Menu_ClientCommand(edict_t *pEntity, const char *arg) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    if (!Menu_IsAuthorized(pEntity)) {
        CLIENT_PRINTF(pEntity, print_center,
                      "REALBOT2: The numpad menu is only available to the listen-server host.");
        return;
    }

    if ((arg == NULL) || (*arg == 0) || (strcmp(arg, "toggle") == 0)) {
        if (menu_page[index] == MENU_PAGE_CLOSED) {
            menu_page[index] = MENU_PAGE_MAIN;
            menu_notice[index][0] = 0;
            Menu_Render(pEntity);
        } else {
            menu_page[index] = MENU_PAGE_CLOSED;
            menu_notice[index][0] = 0;
            CLIENT_PRINTF(pEntity, print_center, "REALBOT2 menu closed.");
        }
        return;
    }

    if ((strcmp(arg, "close") == 0) || (strcmp(arg, "0") == 0)) {
        menu_page[index] = MENU_PAGE_CLOSED;
        menu_notice[index][0] = 0;
        CLIENT_PRINTF(pEntity, print_center, "REALBOT2 menu closed.");
        return;
    }

    if ((strcmp(arg, "+") == 0) || (strcmp(arg, "next") == 0)) {
        Menu_ChangePage(pEntity, menu_page[index] + 1);
        return;
    }

    if ((strcmp(arg, "-") == 0) || (strcmp(arg, "prev") == 0)) {
        Menu_ChangePage(pEntity, menu_page[index] - 1);
        return;
    }

    if (strcmp(arg, "main") == 0) {
        Menu_ChangePage(pEntity, MENU_PAGE_MAIN);
        return;
    }

    if (strcmp(arg, "settings") == 0) {
        Menu_ChangePage(pEntity, MENU_PAGE_SETTINGS);
        return;
    }

    if (strcmp(arg, "team") == 0) {
        Menu_ChangePage(pEntity, MENU_PAGE_TEAM);
        return;
    }

    if (strcmp(arg, "status") == 0) {
        Menu_ChangePage(pEntity, MENU_PAGE_STATUS);
        return;
    }

    if (strcmp(arg, "tools") == 0) {
        Menu_ChangePage(pEntity, MENU_PAGE_TOOLS);
        return;
    }

    if (strcmp(arg, "help") == 0) {
        Menu_PrintHelp(pEntity);
        return;
    }

    if ((arg[0] >= '1') && (arg[0] <= '9') && (arg[1] == 0)) {
        Menu_Select(pEntity, arg[0] - '0');
        return;
    }

    CLIENT_PRINTF(pEntity, print_center,
                  "REALBOT2 menu: unknown option. Use 1-9, +, -, close, or KP_INS.");
}

void Menu_ClientDisconnected(edict_t *pEntity) {
    int index = Menu_Index(pEntity);
    if (index < 0) return;

    menu_page[index] = MENU_PAGE_CLOSED;
    menu_notice[index][0] = 0;
}

void Menu_Reset(void) {
    for (int i = 0; i < 33; i++) {
        menu_page[i] = MENU_PAGE_CLOSED;
        menu_notice[i][0] = 0;
    }
}
