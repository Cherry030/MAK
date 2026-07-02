#pragma once
#include <vrfcgf/cgf.h>
#include <vrfobjcore/simObjectManager.h>
#include <vrfutil/consoleCommandManager.h>
#include <vrfobjcore/localObjectManager.h>



class DtCustomMessageCommand : public DtConsoleCommand
{

private:
	DtSimObjectManager* myObjectManager;

public:
	DtCustomMessageCommand(DtSimObjectManager* objectManager);
	virtual bool execute(const DtString& parameters) override;
};




/*

class DtSetAmmoCommand : public DtConsoleCommand
{
private:
	DtSimObjectManager* myObjectManager;

public:
	DtSetAmmoCommand(DtSimObjectManager* objectManager);
	virtual bool execute(const DtString& parameters) override;
};

class DtSetRoundsRemainingCommand : public DtConsoleCommand
{
private:
	DtSimObjectManager* myObjectManager;

public:
	DtSetRoundsRemainingCommand(DtSimObjectManager* objectManager);
	virtual bool execute(const DtString& parameters) override;
};

class DtForceReloadCommand : public DtConsoleCommand
{
private:
	DtSimObjectManager* myObjectManager;

public:
	DtForceReloadCommand(DtSimObjectManager* objectManager);
	virtual bool execute(const DtString& parameters) override;
};


*/