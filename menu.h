/**
  * REALBOT2 : Numpad menu
  *
  * The menu is intentionally implemented server-side so it works on a
  * listen server without installing any client-side files.  The player
  * binds the numeric keypad keys to "realbot2_menu ..." commands (see
  * cstrike/realbot2_keys.cfg).
  **/

#ifndef REALBOT2_MENU_H
#define REALBOT2_MENU_H

#include <extdll.h>

// Handle a "realbot2_menu" client command.  arg is CMD_ARGV(1) and may
// be NULL/empty ("toggle"), a page name, "+", "-", "close", or 1..9.
void Menu_ClientCommand(edict_t *pEntity, const char *arg);

// Clear the per-client menu state when a client leaves.
void Menu_ClientDisconnected(edict_t *pEntity);

// Clear all menu state (map change / plugin unload).
void Menu_Reset(void);

#endif // REALBOT2_MENU_H
