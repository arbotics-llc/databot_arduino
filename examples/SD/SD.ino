#include <databot.h>

//for now set Serial monitor to no line ending, selection to the left of
//baud selection

//TODO
//handle multiple characters, new line, carriage return, and nlcr properly
//add deletion of files (files should probably be printed to screen then
//  deleted in case deletion is by accident
//check to see if we can clear serial monitor by ASCII char from databot
//build up hierarchical file system
//estimate file time

OpenLog myLog; //Create instance

long totalSize = 0;

void setup()
{
  Wire.begin();
  Wire.setClock(400000); //Go super fast
  myLog.begin(); //Open connection to OpenLog (no pun intended)

  //note, 9600 takes a while for long files... 115200 may be better here
  Serial.begin(115200); //9600bps is used for debug statements
  Serial.println("OpenLog Read File Test");
  
  //first get rid of all those LOG****.TXT files that get generated from somewhere
  Serial.print("Removed files:");
  Serial.println(myLog.removeFile("LOG*.TXT"));

  //we are going to abuse String because we don't have std or stl
  //note: calling any OpenLog member function inbetween searchDirectory() and
  //getNextdirectory() causes problems, this why we build our jank list
  String fileList = "";
  int listIndex = 0;
  #define filesperpage 10
  const int numFiles = createFileList("*", fileList, myLog);
  printFileList(fileList, 100, 0);

  moreFile:
  if(listIndex < 0){
    listIndex = 0;
  }
  Serial.print("list index is");
  Serial.println(listIndex);
  listIndex = printFileList(fileList, filesperpage, listIndex);
  int filesLeft = numFiles - listIndex;

  Serial.print("you have files left:");
  Serial.println(filesLeft);

  Serial.print("Select a file 0 - ");
  Serial.print(filesperpage-1);
  if(filesLeft){
    Serial.print(", more files available, to see more press n");
    if(listIndex > filesperpage){
      Serial.print(", to go back press b");
    }
    Serial.println("");
  }else{
    Serial.println("end of file list, press b to go back");
  }

  while(!Serial.available()){
  }
  uint8_t userinput = Serial.read();
  while(Serial.available()){
    Serial.read();
  }
  if(userinput == 'n'){
     goto moreFile;
  }
  if(userinput == 'b'){
     listIndex = listIndex - filesperpage - (filesperpage % listIndex);
     goto moreFile;
  }
  userinput -= 48;
  Serial.print("you selected file: ");
  Serial.println(userinput);

  String currFile = "";
  int32_t sizeOfFile = -1;
  int j = 0;
  for(int i = 0; i < fileList.length(); i++){
    char c = fileList[i];
    if(c != '\n'){
      currFile += c;
    }else{
      if(j == userinput){
      Serial.print("file ");
      Serial.print(j);
      Serial.print(": ");
      Serial.println(currFile);
      Serial.print("  Size:");
      sizeOfFile = myLog.size(currFile);
      totalSize += sizeOfFile;
      Serial.println(sizeOfFile);
      myLog.readContinuous(sizeOfFile, currFile, i2cBuffHandler);
      }
      j++;
      currFile = "";
    }
  }
}

int createFileList(String filter, String &list, OpenLog &myLog){
  int numFiles = 0;

  myLog.searchDirectory(filter);

  for(String currFile = myLog.getNextDirectoryItem(); currFile != "";currFile = myLog.getNextDirectoryItem()){
    list += currFile;
    list += '\n';
    numFiles++;
  }

  return numFiles;
}

int printFileList(String &list, int filesPerPage, int startIndex){
 String currFile = "";
 int listIndex = 0;
 int numPrinted = 0;
 int32_t sizeOfFile = 0;

 for(int i = 0; i < list.length(); i++){
   char c = list[i];
   if(c != '\n'){
     currFile += c;
   }else{
     if(listIndex >= startIndex){
       numPrinted++;
       Serial.print(listIndex - startIndex);
       Serial.print(": ");
       Serial.println(currFile);
       Serial.print("  Size:");
       sizeOfFile = myLog.size(currFile);
       totalSize += sizeOfFile;
       Serial.println(sizeOfFile);
     }
     currFile = "";
     listIndex++;
     if(numPrinted == filesPerPage){
       break;
     }
   }
 }
 return listIndex;
}

void i2cBuffHandler(uint16_t buffSize,uint8_t *data){
  for(int i = 0; i < buffSize; i++){
    Serial.write(data[i]);
  }
}

void loop()
{
  
}
