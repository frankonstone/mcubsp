/* Retain MCUCOV's generated module descriptors in application flash. */
SECTIONS
{
  mcucov_modules : ALIGN(4)
  {
    __start_mcucov_modules = .;
    KEEP(*(mcucov_modules))
    __stop_mcucov_modules = .;
  } > flash_rodata_seg
}
