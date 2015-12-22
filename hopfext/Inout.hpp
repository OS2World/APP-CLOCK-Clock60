class Inout
{
  HFILE  clockhandle;
  UCHAR  serielle;
public:
  Inout(UCHAR serialPortNr);
  int setInitSequenz();
  void setPortNr(UCHAR serialPortNr);
  int readDate(UCHAR *buffer);
  ~Inout();
  int isok(){return clockhandle;};
  int getPortNr(){return serielle;};
};
