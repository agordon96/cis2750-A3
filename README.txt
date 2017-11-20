Aaron Gordon 0884023
CIS*2750 Assignment 3

Details on the checklist:

FILE MENU - Fully operational.
Open - A dialog is opened, only valid .ics files can be shown, CTRL+O works, creation status is present in the log window, filename is fetched correctly and titled above, getting file works properly.
Save/Save As- Save and save as work perfectly, but I felt that if the validateCalendar returned OK that it would look better to check for OK and then print out "New event created" in the logs. This way the checklist saying "check for printerror" still works and is done, but what the user sees makes much more sense. CTRL+S works for save, and CTRL+A works for save as.
Exit - CTRL+X works, confirmation is present, clicking X does the same, calendar is deleted and freed properly on close.

CREATE MENU - Fully operational.
Create calendar - Calendar object is fully created as expected in C and pointed to in Python.
Create event - Calendar event is put to the end of the list.
Create Alarm - Fully operational.
Delete Event - Present in the create menu. Felt appropriate I guess?

ABOUT MENU - Fully operational.

FILE VIEW - Fully operational.
UI - Rows laid out appropriately, both halves are scrollable (the top one might be hard to check since it has a lot of space but check the bottom for proof) and the event props and alarms can be viewed by double clicking an event on the rows.
Backend - UI is fully populated using an actual Calendar object. If the data is incorrect, it's because the object itself is given incorrectly (not saying my code gives bad objects but the population should work completely).

Create alarm + delete event for 10% bonus and -2% late for +8% bonus total expected
Hopefully it looks nice enough for the other 5% with some custom fonts!

P.S. I had a lot of issues with open, createCalendar had a ton of problems so the code is very messy for me working around it, but then apparently everything was fixed by switching ONE LINE OF CODE which would have made everything easier. Sorry for it being messy, but everything should still work.