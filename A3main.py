#!/usr/bin/python3

from ctypes import *
import os
import tkinter as tk
import tkinter.tix as tix
import tkinter.scrolledtext as scroll
import tkinter.font as tfo
import tkinter.filedialog as tfile
import tkinter.messagebox as tmsg
import tkinter.ttk as tkt

class Calendar(Structure):
  _fields_ = [("version", c_float), ("prodID", c_byte * 1000), ("events", c_void_p), ("properties", c_void_p)]

class Application(tk.Frame):
  def __init__(self, master=None):
    tk.Frame.__init__(self, master)
    self.master.resizable(width=False, height=False)
    self.master.geometry("500x500") 
    self.master.title("iCalGUI")
    self.grid_rowconfigure(0, weight=1)
    self.grid_columnconfigure(0, weight=1)
    
    self.master.protocol("WM_DELETE_WINDOW", self.close)
    self.bind_all("<Control-x>", self.closeWrapper)
    self.bind_all("<Control-o>", self.openWrapper)
    self.bind_all("<Control-s>", self.saveWrapper)
    self.bind_all("<Control-a>", self.saveAsWrapper)
    
    self.parseLib = cdll.LoadLibrary("./bin/libparser.so")
    
    self.printError = self.parseLib.printError
    self.printError.argtypes = [c_int]
    self.printError.restype = c_char_p
    
    self.calOpen = self.parseLib.calOpen
    self.calOpen.argtypes = [c_char_p]
    self.calOpen.restype = POINTER(Calendar)
    
    self.calOpenFail = self.parseLib.calOpenFail
    self.calOpenFail.argtypes = [c_char_p]
    
    self.customCal = self.parseLib.customCal
    self.customCal.argtypes = [(c_char * 1000) * 2]
    self.customCal.restype = POINTER(Calendar)

    self.customEvent = self.parseLib.customEvent
    self.customEvent.argtypes = [POINTER(Calendar), (c_char * 1000) * 4]
    self.customEvent.restype = POINTER(Calendar)
    
    self.customAlarm = self.parseLib.customAlarm
    self.customAlarm.argtypes = [POINTER(Calendar), (c_char * 1000) * 2, c_int]
    self.customAlarm.restype = POINTER(Calendar)
    
    self.getEventNum = self.parseLib.getEventNum
    self.getEventNum.argtypes = [POINTER(Calendar)]
    
    self.getEventArr = self.parseLib.getEventArr
    self.getEventArr.argtypes = [POINTER(Calendar), c_int, POINTER(c_char_p)]
    
    self.writeCalendar = self.parseLib.writeCalendar
    self.writeCalendar.argtypes = [c_char_p, POINTER(Calendar)]
    
    self.validateCalendar = self.parseLib.validateCalendar
    self.validateCalendar.argtypes = [POINTER(Calendar)]
    
    self.deleteEventCal = self.parseLib.deleteEvent
    self.deleteEventCal.argtypes = [POINTER(Calendar), c_int]
        
    self.deleteCalendar = self.parseLib.deleteCalendar
    self.deleteCalendar.argtypes = [POINTER(Calendar)]

    self.printEvent = self.parseLib.printEvent
    self.printEvent.argtypes = [POINTER(Calendar), c_int]
    self.printEvent.restype = c_char_p
    
    self.printError = self.parseLib.printError
    self.printError.argtypes = [c_int]
    self.printError.restype = c_char_p
    
    self.cal = None
    self.filename = ""
    self.eventNum = 0
    
    self.menuFont = tfo.Font(family="Times New Roman", size=12)
    self.topFont = tfo.Font(family="Times New Roman", size=12)
    self.bottomFont = tfo.Font(family="Times New Roman", size=12)
    self.aboutFont = tfo.Font(family="Times New Roman", size=12)
     
    self.createWidgets()
    self.updateScreen()
    
  def closeWrapper(self, event):
    self.close()
    
  def openWrapper(self, event):
    self.openCal()
    
  def saveWrapper(self, event):
    self.saveCal()
    
  def saveAsWrapper(self, event):
    self.saveCalAs()
    
  def updateScreen(self):
    self.tree.delete(*self.tree.get_children())
    if self.eventNum == 0:
      self.createMenu.entryconfig(3, state=tk.DISABLED)
      self.tree.insert("", "end", values=("No Event", "", "", "Please add an event before saving."))
    else:
      self.createMenu.entryconfig(3, state=tk.NORMAL)  
    for i in range(self.eventNum):
      string_buffers = [create_string_buffer(800) for i in range(3)]
      pointers = (c_char_p * 3)(*map(addressof, string_buffers))
      self.getEventArr(self.cal, i, pointers)
      results = [s.value for s in string_buffers]
      self.tree.insert("", "end", values=(str(i + 1), results[0].decode("utf-8"), results[1].decode("utf-8"), results[2].decode("utf-8")))
  
  def deleteEventWrapper(self):
    self.win.destroy()
    
    self.bottomText.config(state=tk.NORMAL)
    self.deleteEventCal(self.cal, self.eventIndex.get())
    
    self.bottomText.insert(tk.INSERT, "Event deleted.\n>> ")
    self.eventNum -= 1
    self.updateScreen()
    self.bottomText.config(state=tk.DISABLED)
  
  def deleteEvent(self):
    self.eventIndex = tk.IntVar()
    
    self.win = tk.Toplevel(self)
    self.win.wm_title("Delete Event")
    
    self.numStrs = []
    for i in range(self.eventNum):
      self.numStrs.append(str(i))
    
    tk.Label(self.win, text="Event #").grid(row=0)
    tkt.Combobox(self.win, textvariable=self.eventIndex, values=self.numStrs).grid(row=0, column=1)
    tk.Button(self.win, text="OK", command=self.deleteEventWrapper).grid(row=2, column=0, sticky=tk.W, pady=4)
    tk.Button(self.win, text="Cancel", command=self.win.destroy).grid(row=2, column=1, sticky=tk.W, pady=4)
  
  def close(self):
    result = tmsg.askyesno("Quit", "Are you sure you'd like to quit?")
    if result:
      if self.cal:
        self.deleteCalendar(self.cal)
      self.quit()

  def showEvent(self, event):
    item = self.tree.identify("item", event.x, event.y)
    eventStr = self.printEvent(self.cal, self.tree.item(self.tree.selection())['values'][0]).decode("utf-8").replace("\t", "")
    
    self.win = tk.Toplevel(self)
    self.win.wm_title("Event Details")
    
    tk.Label(self.win, padx=50, pady=40, font=self.aboutFont, text=eventStr).grid(row=0)
    tk.Button(self.win, text="OK", command=self.win.destroy).grid(row=2, column=0, sticky=tk.S, pady=4)
    
  def about(self):
    self.win = tk.Toplevel(self)
    self.win.wm_title("About")
    
    tk.Label(self.win, padx=50, pady=40, font=self.aboutFont, text="iCalGUI 2017 - Compatible with iCalendar v2.0\nAaron Gordon 2017 - Assignment 3 for CIS*2750").grid(row=0)
    tk.Button(self.win, text="OK", command=self.win.destroy).grid(row=2, column=0, sticky=tk.S, pady=4)
    
  def openCal(self):
    getFile = tfile.askopenfilename(initialdir=os.getcwd(), title="Select .ics file", filetypes = (("iCal files", "*.ics"),))
    if not getFile == "" and getFile:
      self.filename = getFile
      getFile = getFile.split("/")
      self.cal = self.calOpen(self.filename.encode('utf-8'))
      status = 0
      if not self.cal:
        status = self.calOpenFail(self.filename.encode('utf-8'))
      self.bottomText.config(state=tk.NORMAL)
      if not status == 0:
        self.bottomText.insert(tk.INSERT, self.printError(status).decode() + "\n>> ")
      else:
        self.master.title("iCalGUI - " + getFile[-1])
        self.bottomText.insert(tk.INSERT, "File successfully parsed.\n>> ")
        self.fileMenu.entryconfig(1, state=tk.NORMAL)
        self.fileMenu.entryconfig(2, state=tk.NORMAL)
        self.createMenu.entryconfig(1, state=tk.NORMAL)
        self.createMenu.entryconfig(2, state=tk.NORMAL)
        self.eventNum = self.getEventNum(self.cal)
        self.updateScreen()
      self.bottomText.config(state=tk.DISABLED)
    
  def saveCal(self):
    status = self.validateCalendar(self.cal)
    self.bottomText.config(state=tk.NORMAL)
    if self.filename == "" and status == 0:
      self.saveCalAs()
    elif not status == 0:
      self.bottomText.insert(tk.INSERT, self.printError(status).decode() + "\n>> ")
    else:
      self.bottomText.insert(tk.INSERT, "File successfully saved.\n>> ")
      self.writeCalendar(self.filename.encode("utf-8"), self.cal)
    self.bottomText.config(state=tk.DISABLED)

  def saveCalAs(self):
    status = self.validateCalendar(self.cal)
    self.bottomText.config(state=tk.NORMAL)
    if status == 0:
      getFile = tfile.asksaveasfilename(initialdir=os.getcwd(), title="Select .ics file", filetypes=(("iCal files", "*.ics"), ("All files", "*.*")))
      if not getFile == "" and getFile:
        self.filename = getFile
        getFile = getFile.split("/")
        self.master.title("iCalGUI - " + getFile[-1])
        self.bottomText.insert(tk.INSERT, "File successfully saved.\n>> ")
        self.writeCalendar(self.filename.encode("utf-8"), self.cal)
    else:
      self.bottomText.insert(tk.INSERT, self.printError(status).decode() + "\n>> ")    
    self.bottomText.config(state=tk.DISABLED)
    
  def createCalWrapper(self):
    arr = ((c_char * 1000) * 2)()
    self.win.destroy()
    
    self.bottomText.config(state=tk.NORMAL)
    if self.createProd.get() == "":
      self.bottomText.insert(tk.INSERT, "Calendar not created: prodID must not be blank.\n>> ")
    else:
      arr[0].value = b"2.0"
      arr[1].value = self.createProd.get().encode("utf-8")
      self.cal = self.customCal(arr)
      
      self.fileMenu.entryconfig(1, state=tk.NORMAL)
      self.fileMenu.entryconfig(2, state=tk.NORMAL)
      self.createMenu.entryconfig(1, state=tk.NORMAL)
      self.createMenu.entryconfig(2, state=tk.DISABLED)
      
      self.master.title("iCalGUI")
      self.bottomText.insert(tk.INSERT, "New empty calendar created.\n>> ")
      self.eventNum = 0
      self.updateScreen()
    self.bottomText.config(state=tk.DISABLED)
    
  def createCal(self):
    self.createProd = tk.StringVar()
    
    self.win = tk.Toplevel(self)
    self.win.wm_title("Create Calendar")
    
    tk.Label(self.win, text="Product ID").grid(row=0)
    tk.Entry(self.win, textvariable=self.createProd).grid(row=0, column=1)
    tk.Button(self.win, text="OK", command=self.createCalWrapper).grid(row=2, column=0, sticky=tk.W, pady=4)
    tk.Button(self.win, text="Cancel", command=self.win.destroy).grid(row=2, column=1, sticky=tk.W, pady=4)

  def createEventWrapper(self):
    arr = ((c_char * 1000) * 4)()
    self.win.destroy()
    
    self.bottomText.config(state=tk.NORMAL)
    if self.createUID.get() == "":
      self.bottomText.insert(tk.INSERT, "Event not created. UID must not be blank.\n>> ")
    elif not len(self.date.get()) == 8 or not self.date.get().isdigit():
      self.bottomText.insert(tk.INSERT, "Date must be 8 valid numbers.\n>> ")
    elif not len(self.time.get()) == 6 or not self.time.get().isdigit():
      self.bottomText.insert(tk.INSERT, "Time must be 6 valid numbers.\n>> ")
    else:
      arr[0].value = self.createUID.get().encode("utf-8")
      arr[1].value = self.date.get().encode("utf-8")
      arr[2].value = self.time.get().encode("utf-8")
      arr[3].value = str(0).encode("utf-8")

      if self.UTC.get():
        arr[3].value = str(1).encode("utf-8")
      self.cal = self.customEvent(self.cal, arr)  
      
      self.createMenu.entryconfig(2, state=tk.NORMAL)  
      self.bottomText.insert(tk.INSERT, "New event created.\n>> ")
      self.eventNum += 1
      self.updateScreen()
    self.bottomText.config(state=tk.DISABLED)
    
  def createEvent(self):
    self.createUID = tk.StringVar()
    self.date = tk.StringVar()
    self.time = tk.StringVar()
    self.UTC = tk.IntVar()
    
    self.win = tk.Toplevel(self)
    self.win.wm_title("Create Event")
    
    tk.Label(self.win, text="UID").grid(row=0)
    tk.Entry(self.win, textvariable=self.createUID).grid(row=0, column=1)
    tk.Label(self.win, text="Start Date").grid(row=1)
    tk.Entry(self.win, textvariable=self.date).grid(row=1, column=1)
    tk.Label(self.win, text="Start Time").grid(row=2)
    tk.Entry(self.win, textvariable=self.time).grid(row=2, column=1)
    tk.Checkbutton(self.win, text="UTC?", variable=self.UTC).grid(row=3)
    tk.Button(self.win, text="OK", command=self.createEventWrapper).grid(row=5, column=0, sticky=tk.W, pady=4)
    tk.Button(self.win, text="Cancel", command=self.win.destroy).grid(row=5, column=1, sticky=tk.W, pady=4)
  
  def createAlarmWrapper(self):
    arr = ((c_char * 1000) * 2)()
    self.win.destroy()
    
    self.bottomText.config(state=tk.NORMAL)
    if self.action.get() == "" or self.trigger.get() == "":
      self.bottomText.insert(tk.INSERT, "Alarm not created. Action and trigger must be present.\n>> ")
    else:
      arr[0].value = self.action.get().encode("utf-8")
      arr[1].value = self.trigger.get().encode("utf-8")
      self.cal = self.customAlarm(self.cal, arr, self.eventIndex.get())
      
      self.bottomText.insert(tk.INSERT, "New event created.\n>> ")
      self.updateScreen()
    self.bottomText.config(state=tk.DISABLED)
  
  def createAlarm(self):
    self.action = tk.StringVar()
    self.trigger = tk.StringVar()
    self.eventIndex = tk.IntVar()
    
    self.win = tk.Toplevel(self)
    self.win.wm_title("Create Alarm")
    
    self.numStrs = []
    for i in range(self.eventNum):
      self.numStrs.append(str(i))
    
    tk.Label(self.win, text="Action").grid(row=0)
    tk.Entry(self.win, textvariable=self.action).grid(row=0, column=1)
    tk.Label(self.win, text="Trigger").grid(row=1)
    tk.Entry(self.win, textvariable=self.trigger).grid(row=1, column=1)
    tk.Label(self.win, text="Event #").grid(row=2)
    tkt.Combobox(self.win, textvariable=self.eventIndex, values=self.numStrs).grid(row=2, column=1)
    tk.Button(self.win, text="OK", command=self.createAlarmWrapper).grid(row=4, column=0, sticky=tk.W, pady=4)
    tk.Button(self.win, text="Cancel", command=self.win.destroy).grid(row=4, column=1, sticky=tk.W, pady=4)
    
  def createWidgets(self):
    self.top = self.winfo_toplevel()
    
    self.menuBar = tk.Menu(self.top)
    self.menuBar.config(font=self.menuFont)
    self.top['menu'] = self.menuBar

    self.fileMenu = tk.Menu(self.menuBar, tearoff=False)
    self.menuBar.add_cascade(label='File', menu=self.fileMenu)
    self.fileMenu.add_command(label='Open... (CTRL+O)', command=self.openCal)
    self.fileMenu.add_command(label='Save (CTRL+S)', command=self.saveCal, state=tk.DISABLED)
    self.fileMenu.add_command(label='Save as... (CTRL+A)', command=self.saveCalAs, state=tk.DISABLED)
    self.fileMenu.add_command(label='Exit (CTRL+X)', command=self.close)
    
    self.createMenu = tk.Menu(self.menuBar, tearoff=False)
    self.menuBar.add_cascade(label='Create', menu=self.createMenu)
    self.createMenu.add_command(label='Create calendar', command=self.createCal)
    self.createMenu.add_command(label='Create event', command=self.createEvent, state=tk.DISABLED)
    self.createMenu.add_command(label='Create alarm', command=self.createAlarm, state=tk.DISABLED)
    self.createMenu.add_command(label='Delete Event', command=self.deleteEvent, state=tk.DISABLED)
    
    self.helpMenu = tk.Menu(self.menuBar, tearoff=False)
    self.menuBar.add_cascade(label='Help', menu=self.helpMenu)
    self.helpMenu.add_command(label='About iCalGUI...', command=self.about)
    
    self.pane = tk.PanedWindow(orient=tk.VERTICAL)
    self.pane.grid(row=0)
    
    self.tree = tkt.Treeview(columns=("Event #", "Props", "Alarms", "Summary"), height=16, padding=0)
    self.tree.column("#0", minwidth=0, width=0, stretch=tk.NO)
    self.tree.heading("Event #", text="Event #")   
    self.tree.column("Event #", minwidth=0, width=80, stretch=tk.NO)
    self.tree.heading("Props", text="Props")   
    self.tree.column("Props", minwidth=0, width=80, stretch=tk.NO) 
    self.tree.heading("Alarms", text="Alarms")   
    self.tree.column("Alarms", minwidth=0, width=80, stretch=tk.NO) 
    self.tree.heading("Summary", text="Summary                        ")   
    self.tree.column("Summary", minwidth=0)
    self.tree.bind("<Double-1>", self.showEvent)
    
    self.bottomText = scroll.ScrolledText(wrap=tk.WORD, height=12)
    self.pane.add(self.tree)
    self.pane.add(self.bottomText)
    self.bottomText.insert(tk.INSERT, '>> ')
    self.bottomText.config(state=tk.DISABLED)

app = Application()
app.mainloop() 