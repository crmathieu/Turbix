unsigned (far *p)() = 0xffff0000;
unsigned far *sig   = 0x00000472;
main()
{
   *sig = 0x1234;
   (*p)();
}
