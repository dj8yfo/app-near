#include "os.h"
#include "menu.h"

#ifdef HAVE_BAGL

volatile uint8_t dummy_setting_1;
volatile uint8_t dummy_setting_2;

void display_settings(void);
void switch_dummy_setting_1_data(void);
void switch_dummy_setting_2_data(void);

//////////////////////////////////////////////////////////////////////
const char *settings_submenu_getter(unsigned int idx);
void settings_submenu_selector(unsigned int idx);

//////////////////////////////////////////////////////////////////////////////////////
// Enable contract data submenu:

void dummy_setting_1_data_change(unsigned int enabled)
{
  nvm_write((void *)&N_storage.dummy_setting_1, &enabled, 1);
  ui_idle();
}

const char *const dummy_setting_1_data_getter_values[] = {
    "No",
    "Yes",
    "Back"};

const char *dummy_setting_1_data_getter(unsigned int idx)
{
  if (idx < ARRAYLEN(dummy_setting_1_data_getter_values))
  {
    return dummy_setting_1_data_getter_values[idx];
  }
  return NULL;
}

void dummy_setting_1_data_selector(unsigned int idx)
{
  switch (idx)
  {
  case 0:
    dummy_setting_1_data_change(0);
    break;
  case 1:
    dummy_setting_1_data_change(1);
    break;
  default:
    ux_menulist_init(0, settings_submenu_getter, settings_submenu_selector);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Display contract data submenu:

void dummy_setting_2_data_change(unsigned int enabled)
{
  nvm_write((void *)&N_storage.dummy_setting_2, &enabled, 1);
  ui_idle();
}

const char *const dummy_setting_2_data_getter_values[] = {
    "No",
    "Yes",
    "Back"};

const char *dummy_setting_2_data_getter(unsigned int idx)
{
  if (idx < ARRAYLEN(dummy_setting_2_data_getter_values))
  {
    return dummy_setting_2_data_getter_values[idx];
  }
  return NULL;
}

void dummy_setting_2_data_selector(unsigned int idx)
{
  switch (idx)
  {
  case 0:
    dummy_setting_2_data_change(0);
    break;
  case 1:
    dummy_setting_2_data_change(1);
    break;
  default:
    ux_menulist_init(0, settings_submenu_getter, settings_submenu_selector);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Settings menu:

const char *const settings_submenu_getter_values[] = {
    "Dummy setting 1",
    "Dummy setting 2",
    "Back",
};

const char *settings_submenu_getter(unsigned int idx)
{
  if (idx < ARRAYLEN(settings_submenu_getter_values))
  {
    return settings_submenu_getter_values[idx];
  }
  return NULL;
}

void settings_submenu_selector(unsigned int idx)
{
  switch (idx)
  {
  case 0:
    ux_menulist_init_select(0, dummy_setting_1_data_getter, dummy_setting_1_data_selector, N_storage.dummy_setting_1);
    break;
  case 1:
    ux_menulist_init_select(0, dummy_setting_2_data_getter, dummy_setting_2_data_selector, N_storage.dummy_setting_2);
    break;
  default:
    ui_idle();
  }
}

//////////////////////////////////////////////////////////////////////
UX_STEP_NOCB(
    ux_idle_flow_1_step,
    pnn,
    {&C_icon_near,
     "Use wallet to", "view accounts"});
UX_STEP_VALID(
    ux_idle_flow_2_step,
    pb,
    ux_menulist_init(0, settings_submenu_getter, settings_submenu_selector),
    {
        &C_icon_coggle,
        "Settings",
    });
UX_STEP_NOCB(
    ux_idle_flow_3_step,
    bn,
    {
        "Version",
        APPVERSION,
    });
UX_STEP_VALID(
    ux_idle_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
        &C_icon_dashboard_x,
        "Quit",
    });
UX_FLOW(ux_idle_flow,
        &ux_idle_flow_1_step,
        // TODO: Re-enable settings once there are any NEAR-specific ones
        // &ux_idle_flow_2_step,
        &ux_idle_flow_3_step,
        &ux_idle_flow_4_step,
        FLOW_LOOP);

void ui_idle(void)
{
  // reserve a display stack slot if none yet
  if (G_ux.stack_count == 0)
  {
    ux_stack_push();
  }
  ux_flow_init(0, ux_idle_flow, NULL);
}

#endif

#ifdef HAVE_NBGL

#include "nbgl_use_case.h"

void ui_app_quit(void)
{
  os_sched_exit(-1);
}

static const char *const INFO_TYPES[] = {"Version", APPNAME};
static const char *const INFO_CONTENTS[] = {APPVERSION, "(c) 2022 Ledger"};

static bool nav_callback(uint8_t page, nbgl_pageContent_t *content)
{
  UNUSED(page);
  content->type = INFOS_LIST;
  content->infosList.nbInfos = 2;
  content->infosList.infoTypes = (const char **)INFO_TYPES;
  content->infosList.infoContents = (const char **)INFO_CONTENTS;
  return true;
}

void ui_menu_about(void)
{
  nbgl_useCaseSettings(APPNAME, 0, 1, false, ui_idle, nav_callback, NULL);
}

void ui_idle(void)
{
  nbgl_useCaseHome(
      "Test",
      NULL,
      "Tagline",
      false,
      ui_menu_about,
      ui_app_quit);
}

#endif