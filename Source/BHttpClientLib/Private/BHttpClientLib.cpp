/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BHttpClientLib.h"
#include "BLambdaRunnable.h"
#include "BHttpClient.h"
#include "BQueueStream.h"
#include <fstream>

void FBHttpClientLibModule::StartupModule()
{
}
	
IMPLEMENT_MODULE(FBHttpClientLibModule, BHttpClientLib)
