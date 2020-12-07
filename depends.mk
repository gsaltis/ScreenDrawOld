HTTPServer.o: HTTPServer.c HTTPServer.h RPiBaseModules/mongoose.h \
 GeneralUtilities/String.h GeneralUtilities/ANSIColors.h \
 UserInputThread.h
JSONIF.o: JSONIF.c RPiBaseModules/json.h GeneralUtilities/String.h \
 GeneralUtilities/MemoryManager.h JSONIF.h
JSONOut.o: JSONOut.c JSONOut.h GeneralUtilities/String.h \
 GeneralUtilities/MemoryManager.h
main.o: main.c main.h Screen.h GeneralUtilities/String.h ScreenElement.h \
 ScreenElementValue.h JSONOut.h UserInputThread.h WebSocketServer.h \
 RPiBaseModules/mongoose.h HTTPServer.h Version.h \
 GeneralUtilities/ANSIColors.h
Screen.o: Screen.c Screen.h GeneralUtilities/String.h ScreenElement.h \
 ScreenElementValue.h JSONOut.h GeneralUtilities/MemoryManager.h
ScreenElement.o: ScreenElement.c ScreenElement.h ScreenElementValue.h \
 GeneralUtilities/String.h JSONOut.h GeneralUtilities/MemoryManager.h
ScreenElementTemplate.o: ScreenElementTemplate.c ScreenElementTemplate.h \
 GeneralUtilities/String.h ScreenElementValue.h JSONOut.h
ScreenElementValue.o: ScreenElementValue.c ScreenElementValue.h \
 GeneralUtilities/String.h JSONOut.h GeneralUtilities/MemoryManager.h
UserInputThread.o: UserInputThread.c UserInputThread.h \
 GeneralUtilities/String.h RPiBaseModules/linenoise.h \
 GeneralUtilities/ANSIColors.h ScreenElement.h ScreenElementValue.h \
 JSONOut.h GeneralUtilities/MemoryManager.h WebSocketServer.h \
 RPiBaseModules/mongoose.h main.h Screen.h
Version.o: Version.c Version.h
WebConnection.o: WebConnection.c WebConnection.h \
 RPiBaseModules/mongoose.h GeneralUtilities/MemoryManager.h \
 GeneralUtilities/String.h
WebSocketServer.o: WebSocketServer.c WebSocketServer.h \
 RPiBaseModules/mongoose.h JSONOut.h GeneralUtilities/String.h \
 GeneralUtilities/ANSIColors.h GeneralUtilities/MemoryManager.h \
 RPiBaseModules/json.h JSONIF.h HTTPServer.h WebConnection.h
