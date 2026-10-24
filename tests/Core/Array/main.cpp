#include <Core/Array.h>

#include <cstdio>
#include <string>

using namespace NK::Core;

// ---------------------------------------------------------------------------
// Test harness
// ---------------------------------------------------------------------------
static int g_Passed = 0, g_Failed = 0;

#define CHECK(cond) do {                                                  \
    if (cond) { ++g_Passed; }                                             \
    else { ++g_Failed;                                                    \
        std::printf("  FAIL  %s  (line %d)\n", #cond, __LINE__); } } while(0)

#define SECTION(name) std::printf("[%s]\n", name)

// ---------------------------------------------------------------------------
// Leak-tracking type (verifies destructor calls / no leaks)
// ---------------------------------------------------------------------------
struct Tracked {
    static int Alive;
    int Val = 0;

    Tracked() : Val(0) { ++Alive; }
    Tracked(int v) : Val(v) { ++Alive; }
    Tracked(const Tracked& o) : Val(o.Val) { ++Alive; }
    Tracked(Tracked&& o) noexcept : Val(o.Val) { ++Alive; o.Val = -1; }

    Tracked& operator=(const Tracked& o) { Val = o.Val; return *this; }
    Tracked& operator=(Tracked&& o) noexcept { Val = o.Val; o.Val = -1; return *this; }
    ~Tracked() { --Alive; }

    bool operator==(const Tracked& o) const { return Val == o.Val; }
};
int Tracked::Alive = 0;

// ---------------------------------------------------------------------------
// Key-mapped type for IndexOfByKey / FindByKey
// ---------------------------------------------------------------------------
struct Widget {
    int Id = 0;
    std::string Name;
    int Key = 0;

    Widget() = default;
    Widget(int id, const char* name, int key) : Id(id), Name(name), Key(key) {}
    bool operator==(const Widget& o) const { return Id == o.Id; }
};

// ===========================================================================
// Tests
// ===========================================================================

void TestDefaultAndCapacity()
{
    SECTION("TestDefaultAndCapacity");
    {
        Array<int> A;
        CHECK(A.Num() == 0);
        CHECK(A.Max() == 0);
        CHECK(A.IsEmpty());
        CHECK(A.GetData() == nullptr);
        CHECK(A.Slack() == 0);
    }
    {
        Array<int> A(10);
        CHECK(A.Num() == 0);
        CHECK(A.Max() >= 10);
        CHECK(A.IsEmpty());
        CHECK(A.Slack() >= 10);
    }
    {
        Array<int> A(5, 42);
        CHECK(A.Num() == 5);
        for (int i = 0; i < 5; ++i) CHECK(A[i] == 42);
    }
}

void TestInitializerListAndIterators()
{
    SECTION("TestInitializerListAndIterators");
    Array<int> A = { 1, 2, 3, 4, 5 };
    CHECK(A.Num() == 5);
    CHECK(A[0] == 1);
    CHECK(A[4] == 5);
    CHECK(A.Last() == 5);
    CHECK(A.Top() == 5);
    CHECK(A.Last(1) == 4);
    CHECK(*A.At(2) == 3);

    int Sum = 0;
    for (int V : A) Sum += V;
    CHECK(Sum == 15);

    Array<int> B(A.begin(), A.end());
    CHECK(B.Num() == 5);
    CHECK(B[0] == 1 && B[4] == 5);

    Array<int> C(A.GetData() + 1, 3);
    CHECK(C.Num() == 3);
    CHECK(C[0] == 2 && C[1] == 3 && C[2] == 4);
}

void TestCopyAndMove()
{
    SECTION("TestCopyAndMove");
    {
        Array<int> A = { 1, 2, 3 };

        Array<int> B(A);
        CHECK(B.Num() == 3);
        CHECK(B[0] == 1 && B[2] == 3);
        B[0] = 100;
        CHECK(A[0] == 1);            // deep copy

        Array<int> C = A;
        C[1] = 200;
        CHECK(A[1] == 2);

        A = B;
        CHECK(A[0] == 100);

        A = { 7, 8, 9 };
        CHECK(A.Num() == 3 && A[0] == 7 && A[2] == 9);
    }
    {
        Array<std::string> S;
        S.Add("hello");
        S.Add("world");

        Array<std::string> M(std::move(S));
        CHECK(S.Num() == 0);
        CHECK(S.GetData() == nullptr);
        CHECK(M.Num() == 2);
        CHECK(M[0] == "hello");
        CHECK(M[1] == "world");

        Array<std::string> M2;
        M2 = std::move(M);
        CHECK(M.Num() == 0);
        CHECK(M2.Num() == 2);
        CHECK(M2[1] == "world");
    }
}

void TestFindAndContains()
{
    SECTION("TestFindAndContains");
    Array<int> A = { 10, 20, 30, 20, 40 };
    CHECK(A.Contains(20));
    CHECK(A.Contains(40));
    CHECK(!A.Contains(99));

    CHECK(A.Find(20) == 1);
    CHECK(A.FindLast(20) == 3);
    CHECK(A.Find(99) == (Array<int>::TSize) - 1);

    CHECK(A.ContainsByPredicate([](int v) { return v > 35; }));
    CHECK(!A.ContainsByPredicate([](int v) { return v > 100; }));

    CHECK(A.IndexOfByPredicate([](int v) { return v == 30; }) == 2);

    auto* P = A.FindByPredicate([](int v) { return v == 40; });
    CHECK(P != nullptr && *P == 40);

    const auto& CA = A;
    auto* CP = CA.FindByPredicate([](int v) { return v == 10; });
    CHECK(CP != nullptr && *CP == 10);
}

void TestAddVariants()
{
    SECTION("TestAddVariants");
    Array<int> A;
    CHECK(A.Add(10) == 0);
    CHECK(A.Add(20) == 1);
    CHECK(A.AddUnique(20) == 1);       // already present
    CHECK(A.AddUnique(30) == 2);       // appended
    CHECK(A.Num() == 3);

    A.AddZeroed(2);
    CHECK(A.Num() == 5);
    CHECK(A[3] == 0 && A[4] == 0);

    A.AddDefaulted(2);
    CHECK(A.Num() == 7);

    Array<std::string> S;
    S.AddDefaulted(3);
    CHECK(S.Num() == 3);
    CHECK(S[0].empty() && S[2].empty());
}

void TestAppend()
{
    SECTION("TestAppend");
    Array<int> A = { 1, 2, 3 };
    int Extra[] = { 7, 8, 9 };
    A.Append(Extra, 3);
    CHECK(A.Num() == 6);
    CHECK(A[3] == 7 && A[5] == 9);

    A.Append({ 10, 11 });
    CHECK(A.Num() == 8 && A[6] == 10 && A[7] == 11);

    Array<int> B = { 100, 200 };
    A.Append(B);
    CHECK(A.Num() == 10 && A[8] == 100 && A[9] == 200);

    // Self-append (aliasing)
    Array<int> C = { 1, 2, 3 };
    C.Append(C);
    CHECK(C.Num() == 6);
    CHECK(C[0] == 1 && C[3] == 1);
    CHECK(C[2] == 3 && C[5] == 3);

    // Self-append with non-trivial type
    Array<std::string> S;
    S.Add("a"); S.Add("bb"); S.Add("ccc");
    S.Append(S);
    CHECK(S.Num() == 6);
    CHECK(S[0] == "a" && S[3] == "a");
    CHECK(S[2] == "ccc" && S[5] == "ccc");
}

void TestInsert()
{
    SECTION("TestInsert");
    {
        Array<int> A = { 1, 2, 3 };
        A.Insert(99, 1);
        CHECK(A.Num() == 4);
        CHECK(A[0] == 1 && A[1] == 99 && A[2] == 2 && A[3] == 3);

        A.Insert(0, 0);            // front
        CHECK(A[0] == 0 && A[1] == 1);

        A.Insert(42, A.Num());     // end
        CHECK(A.Last() == 42);
    }
    {
        Array<int> A = { 1, 2, 3 };
        int Payload[] = { 100, 200 };
        A.Insert(Payload, 2, 1);
        CHECK(A.Num() == 5);
        CHECK(A[0] == 1 && A[1] == 100 && A[2] == 200 && A[3] == 2 && A[4] == 3);

        A.Insert({ 9, 8 }, 0);
        CHECK(A[0] == 9 && A[1] == 8 && A[2] == 1);

        Array<int> B = { 7, 7 };
        A.Insert(B, A.Num());
        CHECK(A.Last() == 7);
    }
    {
        Array<int> A = { 1, 2, 3 };
        A.InsertZeroed(1, 2);
        CHECK(A.Num() == 5);
        CHECK(A[0] == 1 && A[1] == 0 && A[2] == 0 && A[3] == 2 && A[4] == 3);
    }
    {
        Array<int> A = { 1, 2, 3 };
        A.InsertDefaulted(0, 2);
        CHECK(A.Num() == 5);
        CHECK(A[0] == 0 && A[1] == 0 && A[2] == 1);
    }
}

void TestRemove()
{
    SECTION("TestRemove");
    {
        Array<int> A = { 1, 2, 3, 1, 2, 3, 1 };
        A.Remove(2);                 // Removes a single '2' (MaxCount defaults to 1)
        CHECK(A.Num() == 6);
        CHECK(A[1] == 3);            // first "2" gone, "3" shifted into slot 1

        CHECK(A.Remove(1, 2) == 2);  // Remove up to two "1"s
        CHECK(A.Num() == 4);
    }
    {
        Array<int> A = { 0, 1, 2, 3, 4, 5 };
        CHECK(A.RemoveAt(2) == 1);
        CHECK(A.Num() == 5);
        CHECK(A[2] == 3 && A[3] == 4 && A[4] == 5);

        CHECK(A.RemoveAt(1, 3) == 3);
        CHECK(A.Num() == 2);
        CHECK(A[0] == 0 && A[1] == 5);

        CHECK(A.RemoveAt(10) == 0);
    }
    {
        Array<int> A = { 1, 2, 3, 1, 2, 3 };
        CHECK(A.RemoveAll(2) == 2);
        CHECK(A.Num() == 4);
        CHECK(A[0] == 1 && A[1] == 3 && A[2] == 1 && A[3] == 3);

        CHECK(A.RemoveAll([](int v) { return v == 1; }, true) == 2);
        CHECK(A.Num() == 2);
        CHECK(A[0] == 3 && A[1] == 3);
    }
    {
        // Remove by reference into own storage (aliasing)
        Array<int> A = { 1, 2, 3, 1, 2, 3 };
        A.Remove(A[0], 10);          // Pass a larger max count if you want to remove all copies via aliasing
        CHECK(A.Num() == 4);
        CHECK(A[0] == 2 && A[1] == 3 && A[2] == 2 && A[3] == 3);
    }
}

void TestPop()
{
    SECTION("TestPop");
    Array<int> A = { 10, 20, 30 };
    CHECK(A.Pop() == 30);
    CHECK(A.Num() == 2);

    CHECK(A.Pop(true) == 20);      // with shrink
    CHECK(A.Num() == 1);
    CHECK(A.Max() == 1);
    CHECK(A[0] == 10);
}

void TestResizeAndShrink()
{
    SECTION("TestResizeAndShrink");
    {
        Array<int> A;
        A.Reserve(32);
        CHECK(A.Max() >= 32);
        CHECK(A.Num() == 0);

        A.Add(1); A.Add(2);
        A.SetNum(5);
        CHECK(A.Num() == 5);
        CHECK(A[2] == 0 && A[4] == 0);

        A.SetNum(2);
        CHECK(A.Num() == 2);

        A.SetNumZeroed(4);
        CHECK(A.Num() == 4 && A[2] == 0 && A[3] == 0);

        A.Shrink();
        CHECK(A.Max() == 4);
    }
    {
        Array<int> A = { 1, 2, 3 };
        A.Resize();
        CHECK(A.Num() == 0);

        A.Init(3, 7);
        CHECK(A.Num() == 3 && A[0] == 7 && A[2] == 7);

        A.Empty();
        CHECK(A.Num() == 0);
        CHECK(A.Max() == 0);
    }
}

void TestFilter()
{
    SECTION("TestFilter");
    Array<int> A = { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto Even = A.FilterByPredicate([](int v) { return v % 2 == 0; });
    CHECK(Even.Num() == 4);
    CHECK(Even[0] == 2 && Even[1] == 4 && Even[2] == 6 && Even[3] == 8);
}

void TestKeyBasedLookup()
{
    SECTION("TestKeyBasedLookup");
    Array<Widget> W;
    W.Add(Widget(1, "a", 10));
    W.Add(Widget(2, "b", 20));
    W.Add(Widget(3, "c", 30));

    CHECK(W.IndexOfByKey(20) == 1);
    CHECK(W.IndexOfByKey(999) == (Array<Widget>::TSize) - 1);

    auto* P = W.FindByKey(30);
    CHECK(P != nullptr && P->Id == 3);

    const auto& CW = W;
    auto* CP = CW.FindByKey(10);
    CHECK(CP != nullptr && CP->Id == 1);
}

void TestLeakTracking()
{
    SECTION("TestLeakTracking");
    CHECK(Tracked::Alive == 0);
    {
        Array<Tracked> A;
        for (int i = 0; i < 8; ++i) A.Add(Tracked(i));
        CHECK(A.Num() == 8);
        CHECK(Tracked::Alive == 8);

        A.Reserve(64);              // grow — 8 live, all should still be alive
        CHECK(Tracked::Alive == 8);

        A.SetNum(3);                // destroy 5
        CHECK(A.Num() == 3);
        CHECK(Tracked::Alive == 3);

        A.Insert(Tracked(99), 0);
        CHECK(A.Num() == 4);
        CHECK(Tracked::Alive == 4);

        A.RemoveAt(0);
        CHECK(A.Num() == 3);
        CHECK(Tracked::Alive == 3);

        A.Shrink();
        CHECK(Tracked::Alive == 3);

        A.Empty();
        CHECK(Tracked::Alive == 0);

        for (int i = 0; i < 4; ++i) A.Add(Tracked(i));
        CHECK(Tracked::Alive == 4);
    }
    CHECK(Tracked::Alive == 0);
}

void TestCrossTypeConstruction()
{
    SECTION("TestCrossTypeConstruction");
    Array<int> A = { 1, 2, 3 };

    Array<long long> B(A);          // converting copy
    CHECK(B.Num() == 3);
    CHECK(B[0] == 1 && B[2] == 3);

    Array<long long> C;
    C = A;
    CHECK(C.Num() == 3 && C[1] == 2);

    Array<long long> D(std::move(B));
    CHECK(D.Num() == 3);
    CHECK(B.Num() == 0);
}

// ===========================================================================
// main
// ===========================================================================
int main()
{
    std::printf("=== Array test suite ===\n\n");

    TestDefaultAndCapacity();
    TestInitializerListAndIterators();
    TestCopyAndMove();
    TestFindAndContains();
    TestAddVariants();
    TestAppend();
    TestInsert();
    TestRemove();
    TestPop();
    TestResizeAndShrink();
    TestFilter();
    TestKeyBasedLookup();
    TestLeakTracking();
    TestCrossTypeConstruction();

    std::printf("\n----------------------------------\n");
    std::printf("Result: %d passed, %d failed\n", g_Passed, g_Failed);

    return g_Failed == 0 ? 0 : 1;
}