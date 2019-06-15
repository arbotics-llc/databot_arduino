#include <databot.h>

OpenLog myLog; //Create instance

void setup()
{
  long totalSize = 0;
  Wire.begin();
  Wire.setClock(400000); //Go super fast
  myLog.begin(); //Open connection to OpenLog (no pun intended)

  //note, 9600 takes a while for long files... 115200 may be better here
  Serial.begin(115200); //9600bps is used for debug statements
  Serial.println("OpenLog Read File Test");

  //Record something to the default log
  myLog.println("OpenLog Read File Test");

  String myFile = "longFile.txt";

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
  long startTime = millis();
  Serial.print("Removed files:");
  Serial.println(myLog.removeFile("LOG*.TXT"));
  myLog.searchDirectory("*");
  //we are going to abuse String because we don't have std or stl
  //note: calling any OpenLog member function inbetween searchDirectory() and
  //getNextdirectory() causes problems, this why we build our jank list
  String fileList = "";
  for(String currFile = myLog.getNextDirectoryItem(); currFile != "";currFile = myLog.getNextDirectoryItem()){
    //Serial.println(currFile);
    fileList += currFile;
    fileList += '\n';
    //Serial.println(myLog.size(currFile));
  }
  String currFile = "";
  for(int i = 0; i < fileList.length(); i++){
    char c = fileList[i];
    if(c != '\n'){
      currFile += c;
    }else{
      Serial.println(currFile);
      Serial.print("  Size:");
      sizeOfFile = myLog.size(currFile);
      totalSize += sizeOfFile;
      Serial.println(sizeOfFile);
      myLog.readContinuous(sizeOfFile, currFile, i2cBuffHandler);
      currFile = "";
    }
  }
  long endTime = millis();
  Serial.println("");
  Serial.print("took this long to read erryting: ");
  Serial.println(endTime - startTime);
  Serial.print("and errything was this many bytes: ");
  Serial.println(totalSize);
  
}

void i2cBuffHandler(uint16_t buffSize,uint8_t *data){
  for(int i = 0; i < buffSize; i++){
    Serial.write(data[i]);
  }
}

void loop()
{
  
}
