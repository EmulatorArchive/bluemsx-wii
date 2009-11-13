
int  Trainer_Create(Interface* toolInterface, char* name, int length);
void Trainer_Destroy(void);
void Trainer_Show(void);
void Trainer_NotifyEmulatorStart();
void Trainer_NotifyEmulatorStop();
void Trainer_NotifyEmulatorPause();
void Trainer_NotifyEmulatorResume();
void Trainer_NotifyEmulatorReset();
void Trainer_EmulatorTrace(const char* message);
void Trainer_EmulatorSetBreakpoint(UInt16 slot, UInt16 page, UInt16 address);
void Trainer_SetLanguage(int languageId);
void Trainer_AddArgument(const char* str, void *arg);
const char* Trainer_GetName();

