#pragma once

// A task starved of CPU for longer than this triggers a panic and a reboot
#define WATCHDOG_TIMEOUT_MS 30000

// Call right after pump_init() in app_main. Arms the task watchdog (ESP-IDF 5.1+):
// idle tasks on all cores are watched, and a timeout reboots the board.
void watchdog_init(void);

// Subscribe the CALLING task: it must call watchdog_feed() more often than
// WATCHDOG_TIMEOUT_MS or the board reboots. Use it for long-running loops of your own.
void watchdog_add_task(void);

// Tell the watchdog the calling task is still alive
void watchdog_feed(void);