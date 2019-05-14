#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstdio>
using namespace std;

const int FRAMECOUNT = 256;
const int FRAMESIZE = 256;
const int TLBSIZE = 16;

int main(int argc, char *argv[])
{
char *physicalAddress;
int logicalAddress;
char *frameSpace[FRAMECOUNT];
int pageTable[FRAMECOUNT];
int frameTable[FRAMECOUNT];
int pageNumber;
int frameNumber;
int offset;
int pageMask = 65280;
int offsetMask = 255;
string fileName = argv[1];
ifstream fin;
FILE* bin;

int pageFault = 0;
int pageHit = 0;
int pageTotal = 0;
int TLBhit = 0;
int TLBmiss = 0;
int TLBtotal = 0;
double faultRate;
double TLBrate;
int TLB[16][2];
bool TLBfound;
int currentTLB = 0;

//open files, hopefully
fin.open(fileName.c_str());

if(!fin)
{
cout << "Error, could not open address file." << endl;
}

bin = fopen("BACKING_STORE.bin", "r");

if(!bin)
{
cout << "Error, could not open data file." << endl;
}


//allocate frame memory
for (int i = 0; i < FRAMECOUNT; i++)
{
frameSpace[i] = new char[FRAMESIZE];
}

//initialize page table to empty
for (int i = 0; i < FRAMECOUNT; i++)
{
pageTable[i] = -1;
}

//initialize frame occupancy to empty
for (int i = 0; i < FRAMECOUNT; i++)
{
frameTable[i] = 0;
}



//read file
while(fin >> logicalAddress)
{
//bit mask and shift to get page number
pageNumber = (logicalAddress & pageMask) >> 8;
//bit mask to get offset
offset = logicalAddress & offsetMask;

TLBfound = false;
TLBtotal++;
pageTotal++;

//search TLB first
for (int i = 0; i < TLBSIZE; i++)
{
if (TLB[i][0] == pageNumber)
{
frameNumber = TLB[i][1];
TLBhit++;
TLBfound = true;
}
}

//if TLB fails, search page table
if (!TLBfound)
{

//if located in page table, grab base frame
if (pageTable[pageNumber] != -1)
{
frameNumber = pageTable[pageNumber];
}
//if not in page table, read data and assign to empty frame
else
{
frameNumber = 0;
pageFault++;

//find next empty frame
while (frameTable[frameNumber] != 0)
{
frameNumber++;
}
//set frame to full
frameTable[frameNumber] = 1;
//set page to frame
pageTable[pageNumber] = frameNumber;

//find memory block
fseek(bin, (pageNumber*FRAMESIZE), SEEK_SET);

//read memory block into open frame
fread(frameSpace[frameNumber], 1, FRAMESIZE, bin);
}

//now that frame is found, update TLB
TLB[currentTLB][0] = pageNumber;
TLB[currentTLB][1] = frameNumber;

//if currentTLB count at max size, reset to 0, otherwise increment
if (currentTLB >= (TLBSIZE - 1))
{
currentTLB = 0;
}
else
{
currentTLB++;
}



}

//read and print data
physicalAddress = &frameSpace[frameNumber][offset];

cout << "Logical Address: " << logicalAddress << " Physical Address: " << frameNumber*FRAMESIZE + offset << " Value: " << (int) *physicalAddress << endl;


}

faultRate = (double) pageFault / (double) pageTotal;
TLBrate = (double) TLBhit / (double) TLBtotal;

cout << "Final statistics: " << endl;
cout << "Page-Fault Rate: " << pageFault << "/" << pageTotal << ", " << faultRate * 100 << "%" << endl;
cout << "TLB Hit Rate: " << TLBhit << "/" << TLBtotal << ", " << TLBrate * 100 << "%" << endl;


fin.close();
fclose(bin);

return 0;
}
