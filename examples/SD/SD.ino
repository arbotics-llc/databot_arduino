#include <databot.h>

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

  //Record something to the default log
  myLog.println("OpenLog Read File Test");

  String myFile = "imusensoz.txt";

  myLog.removeFile(myFile);

  //Get size of file
  int32_t sizeOfFile = myLog.size(myFile);
  if (sizeOfFile == -1) //File does not exist. Create it.
  {
    Serial.println(F("File not found, creating one..."));

    myLog.append(myFile); //Create file and begin writing to it

    //Write a random number of random characters to this new file
    myLog.println("The Beginning");
    randomSeed(analogRead(A0));

    //Write 300 to 500 random characters to the file
    int charsToWrite = random(10000, 20000);
    for (int x = 0 ; x < charsToWrite ; x++)
    {
      byte myCharacter = random('a', 'z'); //Pick a random letter, a to z
      //line breaks, yo
      if(x%80 == 0){
        myCharacter = '\n';
      }

      myLog.write(myCharacter);
    }
    myLog.println();
    myLog.println("The End");
    myLog.syncFile();
  }
  else
  {
    Serial.println("File found!");
  }

  //Get size of file
  sizeOfFile = myLog.size(myFile);

  if (sizeOfFile > -1)
  {
    Serial.print(F("Size of file: "));
    Serial.println(sizeOfFile);

    //Read the contents of myFile by passing a buffer into .read()
    //Then printing the contents of that buffer
    int32_t myBufferSize = 0;
    byte myBuffer[myBufferSize];
    //myLog.read(myBuffer, myBufferSize, myFile, 4); //Doesn't yet work
    myLog.readContinuous(sizeOfFile, myFile, i2cBuffHandler); //Load myBuffer with contents of myFile
    Serial.println("\nDone with file contents");
  }
  else
  {
    Serial.println(F("Size error: File not found"));
  }

  Serial.println(F("Done!"));

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
  int j = 0;
  for(int i = 0; i < fileList.length(); i++){
    char c = fileList[i];
    if(c != '\n'){
      currFile += c;
    }else{
      j++;
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
