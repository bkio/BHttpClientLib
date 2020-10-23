/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BHttpClient.h"
#include <iostream>
#include "BHttpClientUtils.h"
#include "GenericPlatform/GenericPlatformHttp.h"

BHTTPCLIENTLIB_API DEFINE_LOG_CATEGORY(LogBHttpClientLib);

enum BHTTPCLIENTLIB_API EBHttpCreateUpdateMethod : uint8
{
    Post = 0,
    Put = 1,
    Patch = 2
};

enum BHTTPCLIENTLIB_API EBHttpReadDeleteMethod : uint8
{
    Get = 0,
    Delete = 1
};

/* 
 * Analyze the full path for extracting the information of host, path and ssl client needed
 * 
 * */
void BHttpClient::SplitPath(const FString& FullPath, FString& HostOnly, FString& PathOnly)
{
    bool SSLSupport = false;
    FString FullPathWithProtocol;
    if (FullPath.StartsWith(TEXT("https://")))
    {
        SSLSupport = true;
        FullPathWithProtocol = FullPath;
    }
	else if (!FullPath.StartsWith(TEXT("http://")))
	{
		FullPathWithProtocol = TEXT("http://") + FullPath;
	}
    else
    {
        FullPathWithProtocol = FullPath;
    }

    FString FullPathWithoutProtocol;
    if (SSLSupport)
    {
        FullPathWithoutProtocol = FullPathWithProtocol.Replace(TEXT("https://"), TEXT(""));
    }
    else
    {
        FullPathWithoutProtocol = FullPathWithProtocol.Replace(TEXT("http://"), TEXT(""));
    }

    int32 firstIdx = FullPathWithoutProtocol.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
    if (firstIdx < 0)
    {
        HostOnly = FullPathWithoutProtocol;
        PathOnly = "/";
    }
    else
    {
        HostOnly = FullPathWithoutProtocol.Mid(0, firstIdx);
        PathOnly = FullPathWithoutProtocol.Mid(firstIdx);
    }
}

/*
 * Get_Or_Delete method handles Get and Delete requests
 * 
 * */
int32 BHttpClient::Get_Or_Delete(EBHttpReadDeleteMethod HttpMethod, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData)
{
	int32 Result = -1;
	int32 RetryCount = 0;

	do
	{
		Result = Get_Or_Delete_Internal(HttpMethod, OutputStream, Host, Path, HeadersData);
	} 
    while (Result == -1 && RetryCount++ < 10 && SleepInternal(1.0f));

	return Result;
}
int32 BHttpClient::Get_Or_Delete_Internal(EBHttpReadDeleteMethod HttpMethod, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData)
{
    // Converting TMap Headers data to httplib::Headers as std::multimap 
    httplib::Headers headers;
    if (HeadersData.Num() > 0)
    {
        TArray<FString> KeyArray;
        HeadersData.GenerateKeyArray(KeyArray);

        TArray<FString> ValueArray;
        HeadersData.GenerateValueArray(ValueArray);

        for (int i = 0; i < HeadersData.Num(); i++)
        {
            headers.emplace(std::make_pair(std::string(TCHAR_TO_UTF8(*KeyArray[i])), std::string(TCHAR_TO_UTF8(*ValueArray[i]))));
        }
    }

    // ResponseHandler definition for handling response message after sending Get or Delete requests
    httplib::ResponseHandler response_handler;
    response_handler = [&](const httplib::Response& response) {
        UE_LOG(LogBHttpClientLib, Warning, TEXT("HttpClient ResponseHandler Get/Delete ==> Status: %d"), response.status);
        return true; // return 'false' if you want to cancel the request.
    };

    // ContentReceiver definition for writing the ostream based on read data and length
    httplib::ContentReceiver content_receiver;
    if (OutputStream)
    {
		content_receiver = [OutputStream](const char* data, size_t data_length) {
			OutputStream->write(data, data_length);
			return true;
		};
    }
    
    // Progress definition for getting progress information about receiving data
    httplib::Progress progress_tracker;
    progress_tracker = [](uint64_t len, uint64_t total) {
        UE_LOG(LogBHttpClientLib, Warning, TEXT("HttpClient Progress Get/Delete ===> %lld / %lld bytes (%d%% complete)\n"), len, total, (int)(len * 100 / total));
        return true; // return 'false' if you want to cancel the request.
    };

    // Storing result messages
    int ResponseStatusCode = -1;

    httplib::Client normalclient(TCHAR_TO_UTF8(*Host));
    normalclient.set_keep_alive(true);
    if (HttpMethod == EBHttpReadDeleteMethod::Delete)
    {
        auto result = normalclient.Delete(TCHAR_TO_UTF8(*Path), headers, response_handler, content_receiver, progress_tracker);
        if (result)
        {
            ResponseStatusCode = result->status;
        }
    }
    else
    {
        auto result = normalclient.Get(TCHAR_TO_UTF8(*Path), headers, response_handler, content_receiver, progress_tracker);
        if (result)
        {
            ResponseStatusCode = result->status;
        }
    }

    return ResponseStatusCode;
}

int32 BHttpClient::Get(std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData)
{
    FString HostOnly;
    FString PathOnly;
    BHttpClient::SplitPath(FullPath, HostOnly, PathOnly);

    return BHttpClient::Get_Or_Delete(EBHttpReadDeleteMethod::Get, OutputStream, HostOnly, PathOnly, HeadersData);
}

int32 BHttpClient::Get(std::ostream* OutputStream, const FString& FullPath)
{
    TMap<FString, FString> HeadersData;
    return BHttpClient::Get(OutputStream, FullPath, HeadersData);
}

int32 BHttpClient::Delete(std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData)
{
    FString HostOnly;
    FString PathOnly;
    BHttpClient::SplitPath(FullPath, HostOnly, PathOnly);

    return BHttpClient::Get_Or_Delete(EBHttpReadDeleteMethod::Delete, OutputStream, HostOnly, PathOnly, HeadersData);
}

int32 BHttpClient::Delete(std::ostream* OutputStream, const FString& FullPath)
{
    TMap<FString, FString> HeadersData;
    return BHttpClient::Delete(OutputStream, FullPath, HeadersData);
}

/*
 * POST/PUT/PATCH METHODS IMPLEMENTATIONS
 **/
int32 BHttpClient::Post_Or_Put_Or_Patch(EBHttpCreateUpdateMethod HttpMethod, std::istream* InputStream, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    int32 Result = -1;
    int32 RetryCount = 0;

    do
    {
        Result = Post_Or_Put_Or_Patch_Internal(HttpMethod, InputStream, OutputStream, Host, Path, HeadersData, ContentType, FormData);
	}
	while (Result == -1 && RetryCount++ < 10 && SleepInternal(1.0f));

    return Result;
}
int32 BHttpClient::Post_Or_Put_Or_Patch_Internal(EBHttpCreateUpdateMethod HttpMethod, std::istream* InputStream, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    // Converting TMap Headers data to httplib::Headers as std::multimap
    httplib::Headers headers;
    if (HeadersData.Num() > 0)
    {
        TArray<FString> KeyArray;
        HeadersData.GenerateKeyArray(KeyArray);

        TArray<FString> ValueArray;
        HeadersData.GenerateValueArray(ValueArray);

        for (int i = 0; i < HeadersData.Num(); i++)
        {
            headers.emplace(std::make_pair(std::string(TCHAR_TO_UTF8(*KeyArray[i])), std::string(TCHAR_TO_UTF8(*ValueArray[i]))));
        }
    }

    // Converting TMap FormData data to httplib::Params as std::multimap
    httplib::Params params;
    if (FormData.Num() > 0)
    {
        TArray<FString> KeyArray;
        FormData.GenerateKeyArray(KeyArray);

        TArray<FString> ValueArray;
        FormData.GenerateValueArray(ValueArray);

        for (int i = 0; i < FormData.Num(); i++)
        {
            params.emplace(std::make_pair(std::string(TCHAR_TO_UTF8(*KeyArray[i])), std::string(TCHAR_TO_UTF8(*ValueArray[i]))));
        }
    }

    // ContentProvider definition for providing istream data to the server
    httplib::ContentProvider content_provider;

    size_t StreamSize = 0;

    if (InputStream)
    {
        content_provider = [InputStream](size_t offset, size_t length, httplib::DataSink& sink) {
            do
            {
                char buffer[CPPHTTPLIB_RECV_BUFSIZ];
                InputStream->read(buffer, CPPHTTPLIB_RECV_BUFSIZ);
                //read correct in first pass but fail in second one
                unsigned int readBytes = InputStream->gcount();
                if (readBytes > 0)
                {
                    sink.write(buffer + offset, readBytes);
                    UE_LOG(LogBHttpClientLib, Warning, TEXT("HttpClient ContentProvider Post/Put/Patch ==> Written Bytes: %lld"), InputStream->gcount());
                }
            } while (InputStream->gcount() > 0);

            sink.done();

            return true;
        };
    }

    // ResponseHandler definition for handling response message after sending Post/Put/Patch requests
    httplib::ResponseHandler response_handler;
    response_handler = [&](const httplib::Response& response) {
        UE_LOG(LogBHttpClientLib, Warning, TEXT("HttpClient ResponseHandler Post/Put/Patch ==> Status: %d"), response.status);
        return true; // return 'false' if you want to cancel the request.
    };

    // ContentReceiver definition for writing the ostream based on read data and length
    httplib::ContentReceiver content_receiver;
    if (OutputStream)
    {
        content_receiver = [OutputStream](const char* data, size_t data_length) {
            OutputStream->write(data, data_length);
            return true;
        };
    }
    

    // Progress definition for getting progress information about receiving data
    httplib::Progress progress_tracker;
    progress_tracker = [](uint64_t len, uint64_t total) {
        UE_LOG(LogBHttpClientLib, Warning, TEXT("HttpClient Progress Post/Put/Patch ===> %lld / %lld bytes (%d%% complete)\n"), len, total, (int)(len * 100 / total));
        return true; // return 'false' if you want to cancel the request.
    };

    // Storing result messages
    int ResponseStatusCode = -1;

    // If StreamSize is equal to zero, istream is empty or cannot be read
    httplib::Client normalclient(TCHAR_TO_UTF8(*Host));
    normalclient.set_keep_alive(true);
    if (HttpMethod == EBHttpCreateUpdateMethod::Post)
    {
        auto result = normalclient.Post(TCHAR_TO_UTF8(*Path), headers, params, StreamSize, content_provider, TCHAR_TO_UTF8(*ContentType), response_handler, content_receiver, progress_tracker);
        if (result)
        {
            ResponseStatusCode = result->status;
        }
    }
    else if (HttpMethod == EBHttpCreateUpdateMethod::Put)
    {
        auto result = normalclient.Put(TCHAR_TO_UTF8(*Path), headers, params, StreamSize, content_provider, TCHAR_TO_UTF8(*ContentType), response_handler, content_receiver, progress_tracker);
        if (result)
        {
            ResponseStatusCode = result->status;
        }
    }
    else if (HttpMethod == EBHttpCreateUpdateMethod::Patch)
    {
        auto result = normalclient.Patch(TCHAR_TO_UTF8(*Path), headers, params, StreamSize, content_provider, TCHAR_TO_UTF8(*ContentType), response_handler, content_receiver, progress_tracker);
        if (result)
        {
            ResponseStatusCode = result->status;
        }
    }
    
    return ResponseStatusCode;
}

int32 BHttpClient::Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    FString HostOnly;
    FString PathOnly;
    BHttpClient::SplitPath(FullPath, HostOnly, PathOnly);

    return BHttpClient::Post_Or_Put_Or_Patch(EBHttpCreateUpdateMethod::Post, InputStream, OutputStream, HostOnly, PathOnly, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType)
{
    TMap<FString, FString> FormData;
    return BHttpClient::Post(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    TMap<FString, FString> HeadersData;
    return BHttpClient::Post(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType)
{
    TMap<FString, FString> HeadersData;
    TMap<FString, FString> FormData;
    return BHttpClient::Post(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    FString HostOnly;
    FString PathOnly;
    BHttpClient::SplitPath(FullPath, HostOnly, PathOnly);

    return BHttpClient::Post_Or_Put_Or_Patch(EBHttpCreateUpdateMethod::Put, InputStream, OutputStream, HostOnly, PathOnly, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType)
{
    TMap<FString, FString> FormData;
    return BHttpClient::Put(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    TMap<FString, FString> HeadersData;
    return BHttpClient::Put(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType)
{
    TMap<FString, FString> HeadersData;
    TMap<FString, FString> FormData;
    return BHttpClient::Put(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    FString HostOnly;
    FString PathOnly;
    BHttpClient::SplitPath(FullPath, HostOnly, PathOnly);

    return BHttpClient::Post_Or_Put_Or_Patch(EBHttpCreateUpdateMethod::Patch, InputStream, OutputStream, HostOnly, PathOnly, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType)
{
    TMap<FString, FString> FormData;
    return BHttpClient::Patch(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData)
{
    TMap<FString, FString> HeadersData;
    return BHttpClient::Patch(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

int32 BHttpClient::Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType)
{
    TMap<FString, FString> HeadersData;
    TMap<FString, FString> FormData;
    return BHttpClient::Patch(InputStream, OutputStream, FullPath, HeadersData, ContentType, FormData);
}

bool BHttpClient::SleepInternal(float InSeconds)
{
    FPlatformProcess::Sleep(InSeconds);
    return true;
}