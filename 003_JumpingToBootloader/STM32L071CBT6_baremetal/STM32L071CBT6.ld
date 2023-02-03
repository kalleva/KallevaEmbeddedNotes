_estack = 0x20005000;

MEMORY
{
    FLASH ( rx )      : ORIGIN = 0x08000000, LENGTH = 192K
    RAM ( rxw )       : ORIGIN = 0x20000000, LENGTH = 20K
}