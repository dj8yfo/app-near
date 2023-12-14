#ifdef HAVE_NBGL

#include "menu.h"
#include "globals.h"
#include "nbgl_use_case.h"


//  ----------------------------------------------------------- 
//  --------------------- SETTINGS MENU -----------------------
//  ----------------------------------------------------------- 
static const char* const INFO_TYPES[] = {"Version", "Developer"};
static const char* const INFO_CONTENTS[] = {APPVERSION, "NEAR Protocol"};

#define MAX_STRING_LENGTH 100

static char settings_title[MAX_STRING_LENGTH] = {0};

enum {
    BLIND_SIGN_SWITCH_IDX = 0,
    SWITCHES_TOTAL,
};
static nbgl_layoutSwitch_t G_switches[SWITCHES_TOTAL];

enum {
    BLIND_SIGN_SWITCH_TOKEN = FIRST_USER_TOKEN,
};

#define SETTINGS_PAGES_TOTAL 2

static bool settings_nav_callback(uint8_t page, nbgl_pageContent_t *content)
{
  if (page == 0) {
    content->type = INFOS_LIST;
    content->infosList.nbInfos = 2;
    content->infosList.infoTypes = INFO_TYPES;
    content->infosList.infoContents = INFO_CONTENTS;
    return true;
  } else if (page == 1) {

    if (blind_sign_enabled == BlindSignDisabled) {
        G_switches[BLIND_SIGN_SWITCH_IDX].initState = OFF_STATE;
    } else {
        G_switches[BLIND_SIGN_SWITCH_IDX].initState = ON_STATE;
    }

    content->type = SWITCHES_LIST;
    content->switchesList.nbSwitches = SWITCHES_TOTAL;
    content->switchesList.switches = G_switches;
    return true;
    
  } else {
    return false;  
  }
}

static void settings_controls_callback(int token, uint8_t index) {
    UNUSED(index);
    uint8_t value;
    switch (token) {
        case BLIND_SIGN_SWITCH_TOKEN:
            value = (G_switches[BLIND_SIGN_SWITCH_IDX].initState != ON_STATE);
            nvm_write((void *)&N_storage.blind_sign_enabled, &value, sizeof(value));
            blind_sign_enabled = value;
            break;
        default:
            PRINTF("Unreachable in `settings_controls_callback`\n");
            break;
    }
}
// info menu definition
void ui_menu_settings(void)
{
  #define INIT_INFO_PAGE (0)

  G_switches[BLIND_SIGN_SWITCH_IDX].text = "Blind Sign";
  G_switches[BLIND_SIGN_SWITCH_IDX].subText = "Enable blind signing";
  G_switches[BLIND_SIGN_SWITCH_IDX].token = BLIND_SIGN_SWITCH_TOKEN;
  G_switches[BLIND_SIGN_SWITCH_IDX].tuneId = TUNE_TAP_CASUAL;

  strlcpy(settings_title, APPNAME, MAX_STRING_LENGTH);
  strlcat(settings_title, " Settings", MAX_STRING_LENGTH);
  nbgl_useCaseSettings(settings_title, INIT_INFO_PAGE, SETTINGS_PAGES_TOTAL, false, ui_idle, settings_nav_callback, settings_controls_callback);
}

//  ----------------------------------------------------------- 
//  --------------------- HOME SCREEN MENU --------------------
//  ----------------------------------------------------------- 
void ui_app_quit(void)
{
  os_sched_exit(-1);
}


// home page defintion
void ui_idle(void)
{
  #define SETTINGS_BUTTON_DISABLED (false)

  nbgl_useCaseHome(
      APPNAME,
      &C_stax_app_near_64px,
      NULL,
      SETTINGS_BUTTON_DISABLED,
      ui_menu_settings,
      ui_app_quit);
}



#endif
