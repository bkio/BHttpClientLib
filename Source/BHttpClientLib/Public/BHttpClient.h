/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

BHTTPCLIENTLIB_API DECLARE_LOG_CATEGORY_EXTERN(LogBHttpClientLib, Log, All);

enum BHTTPCLIENTLIB_API EBHttpCreateUpdateMethod : uint8;
enum BHTTPCLIENTLIB_API EBHttpReadDeleteMethod : uint8;

class BHTTPCLIENTLIB_API BHttpClient
{
public:

    //************************************
    // Method:    SplitPath is a function that split Host and Path
    // FullName:  BHttpClient::SplitPath
    // Access:    public static 
    // Returns:   int32
    // Qualifier:
    // Parameter: const FString & FullPath
    // Parameter: FString & HostOnly
    // Parameter: FString & PathOnly
    //************************************
    static void SplitPath(const FString& FullPath, FString& HostOnly, FString& PathOnly);

    static int32 Get(std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData);

    static int32 Get(std::ostream* OutputStream, const FString& FullPath);

    static int32 Delete(std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData);

    static int32 Delete(std::ostream* OutputStream, const FString& FullPath);


    static int32 Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData);

    static int32 Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType);

    static int32 Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData);
    
    static int32 Post(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType);


    static int32 Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData);

    static int32 Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType);

    static int32 Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData);

    static int32 Put(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType);


    static int32 Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData);

    static int32 Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const TMap<FString, FString>& HeadersData, const FString& ContentType);

    static int32 Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType, const TMap<FString, FString>& FormData);

    static int32 Patch(std::istream* InputStream, std::ostream* OutputStream, const FString& FullPath, const FString& ContentType);

private:
    //************************************
    // Method:    Get_Or_Delete to handle Get and Delete requests extracts ostream for downloading the response
    // FullName:  BHttpClient::Get_Or_Delete
    // Access:    private static 
    // Returns:   int32
    // Qualifier:
    // Parameter: EBHttpReadDeleteMethod HttpMethod
    // Parameter: std::ostream & OutputStream
    // Parameter: const FString & Host
    // Parameter: const FString & Path
    // Parameter: const TMap<FString
    // Parameter: FString> & HeadersData
    //************************************
    static int32 Get_Or_Delete(EBHttpReadDeleteMethod HttpMethod, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData);
    static int32 Get_Or_Delete_Internal(EBHttpReadDeleteMethod HttpMethod, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData);

    //************************************
    // Method:    Post_Or_Put_Or_Patch to handle Post/Put/Patch requests with istream and extracts ostream if there is available output from server
    // FullName:  BHttpClient::Post_Or_Put_Or_Patch
    // Access:    private static 
    // Returns:   int32
    // Qualifier:
    // Parameter: EBHttpCreateUpdateMethod HttpMethod
    // Parameter: std::istream & InputStream
    // Parameter: std::ostream & OutputStream
    // Parameter: const FString & Host
    // Parameter: const FString & Path
    // Parameter: const TMap<FString
    // Parameter: FString> & HeadersData
    // Parameter: const FString & ContentType
    // Parameter: const TMap<FString
    // Parameter: FString> & FormData
    //************************************
    static int32 Post_Or_Put_Or_Patch(EBHttpCreateUpdateMethod HttpMethod, std::istream* InputStream, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData);
    static int32 Post_Or_Put_Or_Patch_Internal(EBHttpCreateUpdateMethod HttpMethod, std::istream* InputStream, std::ostream* OutputStream, const FString& Host, const FString& Path, const TMap<FString, FString>& HeadersData, const FString& ContentType, const TMap<FString, FString>& FormData);

    static bool SleepInternal(float InSeconds);
};