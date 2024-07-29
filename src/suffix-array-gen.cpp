#if 1
// C++ program for building suffix array of a given text
#include <algorithm>
#include <cstring>
#include <iostream>

#include <windows.h>

#include <pathcch.h>

using namespace std;

// Structure to store information of a suffix
struct suffix
{
    union
    {
        struct
        {
            uint32_t next;
            uint32_t rank; // To store ranks and next rank pair
        };
        uint64_t value;
    };
};

static_assert(sizeof(suffix) == 8, "");

// A utility function to print an array of given size
void printArr(int arr[], uint64_t arr_value[], int n)
{
    for (int i = 0; i < n; i++)
        cout << arr[i] << "," << arr_value[arr[i]] << " ";
    cout << endl;
}

void printArr(int arr[], int n)
{
    for (int i = 0; i < n; i++)
        cout << arr[i] << " ";
    cout << endl;
}

// This is the main function that takes a string 'txt' of size n as an
// argument, builds and return the suffix array for the given string
void buildSuffixArray(int *su_index, struct suffix *payload, char *txt, int n)
{
    // A structure to store suffixes and their indexes

    // Store suffixes and their indexes in an array of structures.
    // The structure is needed to sort the suffixes alphabetically
    // and maintain their old indexes while sorting
    int prev = 0;
    for (int i = n - 1; i >= 0; --i)
    {
        su_index[i] = i;
        payload[i].next = prev;
        prev = (unsigned char)txt[i];
        payload[i].rank = prev;
    }
    auto cmp_lambda = [=](int a, int b) { return payload[a].value < payload[b].value; };
    // Sort the suffixes using the comparison function
    // defined above.
    std::sort(su_index, su_index + n, cmp_lambda);
    // printArr(su_index, (uint64_t *)payload, n);

    // At this point, all suffixes are sorted according to first
    // 2 characters.  Let us sort suffixes according to first 4
    // characters, then first 8 and so on
    for (int k = 4; k < 2 * n; k <<= 1)
    {
        double time = (double)(clock()) / CLOCKS_PER_SEC;
        printf("The current time:%lf k:%d\n", time, k);
        // Assigning rank and index values to first suffix
        int rank = 0;
        int prev = payload[su_index[0]].rank;
        payload[su_index[0]].rank = rank;
        // Assigning rank to suffixes
        for (int i = 1; i < n; i++)
        {
            auto index_i = su_index[i];
            // If first rank and next ranks are same as that of previous
            // suffix in array, assign the same new rank to this suffix
            auto rank_i_fetched = payload[index_i].rank;
            if (rank_i_fetched == prev &&
                payload[index_i].next == payload[su_index[i - 1]].next)
            {
                payload[index_i].rank = rank;
            }
            else // Otherwise increment rank and assign
            {
                payload[index_i].rank = ++rank;
            }
            prev = rank_i_fetched;
        }

        // Assign next rank to every suffix
        for (int i = 0; i < n; i++)
        {
            int nextP = su_index[i] + (k >> 1);
            if (nextP < n)
            {
                payload[su_index[i]].next = payload[nextP].rank;
            }
            else
            {
                payload[su_index[i]].next = 0;
            }
        }

        // Sort the suffixes according to first k characters
        std::sort(su_index, su_index + n, cmp_lambda);
        // printArr(su_index, (uint64_t *)payload, n);
    }
}
// Driver program to test above functions
int main()
{
    auto f = fopen("docs/win32-api-merged.h.txt", "rb");
    auto f_output = fopen("docs/win32-api-merged.h.index.txt", "wb");
    if (f == nullptr)
    {
        puts("open docs/win32-api-merged.h.txt failed\n");
        return -1;
    }

    puts("");
    puts("|         n | Runtime(s) |    TPI(ms)   |");
    fseek(f, 0, SEEK_END);
    int n = ftell(f) + 1;
    printf("The size is %d\n", n);
    fseek(f, 0, SEEK_SET);
    {
#if 1
        char *txt = new char[n];
        txt[n] = 0;
        txt[n - 1] = 1;
        fread(txt, 1, n - 1, f);
        printf("The file read finished\n");
#else
        char txt[] = "banana";
        n = strlen(txt);
#endif

        auto suffixArr = new int[n];
        auto payload = new suffix[n];
        time_t start = clock();
        buildSuffixArray(suffixArr, payload, txt, n);
        time_t stop = clock();
        double time = (double)(stop - start) / CLOCKS_PER_SEC;
        double cpi = (double)(stop - start) / (n * log2(n));
        puts("|----------:|:----------:|:------------:|");
        printf("| `%7d` |   `%5.3f`  | `%0.8f` |\n", n, time, cpi);
        fwrite(suffixArr, sizeof(int), n, f_output);
#if 1
        delete txt;
#else
        printArr(suffixArr, n);
#endif
        delete payload;
        delete suffixArr;
    }
    fclose(f);
    fclose(f_output);

    // Following is suffix array for banana
    // 5 3 1 0 4 2
    // printArr(suffixArr, n);
    return 0;
}
#else

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;
/*------------------------------------------------------------------------------------*/

void print(int k, const char *title);
bool test = false;

int n;            // text length
char *T;          // text string
int *SA, *tempSA; // the sorted suffixes
int *RA, *tempRA; // ranks of suffix array
int *L;           // used in counting sort

inline int getRA(int i) { return (i < n) ? RA[i] : 0; }

void radix_sort(int k)
{
    memset(L, 0, sizeof(int) * n);
    // count frequencies
    for (int i = 0; i < n; ++i)
    {
        L[getRA(i + k)]++;
    }
    // save first index of every characters
    int mx = n > 130 ? n : 130;
    for (int i = 0, s = 0; i < mx; ++i)
    {
        int x = L[i];
        L[i] = s;
        s += x;
    }
    // build sorted tempSA
    for (int i = 0; i < n; ++i)
    {
        int &x = L[getRA(SA[i] + k)];
        tempSA[x++] = SA[i];
    }
    // copy tempSA to SA
    for (int i = 0; i < n; ++i)
    {
        SA[i] = tempSA[i];
    }
}
// text must ends with a $
void buildSA()
{
    // initialize
    n = strlen(T);
    T[n++] = '\x01', T[n] = 0; // append $
    for (int i = 0; i < n; ++i)
    {
        SA[i] = i;
        RA[i] = T[i];
    }
    if (!test)
        print(1, "Initialized:");

    // algorithm loop
    for (int k = 1; k < n; k <<= 1)
    {
        double time = (double)(clock()) / CLOCKS_PER_SEC;
        printf("The current time:%lf\n", time);
        // sort by k-th ranks
        radix_sort(k);
        radix_sort(0);
        if (!test)
            print(k, "After sorting:");
        // compute new ranks
        tempRA[SA[0]] = 0;
        for (int i = 1, r = 0; i < n; ++i)
        {
            if (getRA(SA[i - 1]) != getRA(SA[i]))
            {
                r++;
            }
            else if (getRA(SA[i - 1] + k) != getRA(SA[i] + k))
            {
                r++;
            }
            tempRA[SA[i]] = r;
        }
        for (int i = 0; i < n; ++i)
        {
            RA[i] = tempRA[i];
        }
        if (!test)
            print(k, "New ranks:");
        if (RA[SA[n - 1]] == n - 1)
            break;
    }
}

void print(int k, const char *title = "")
{
    if (title[0])
        printf("%s\n", title);
    puts("========================================================");
    printf("|  i | SA[i] | RA[SA[i]] | RA[SA[i] +%2d] | tempRA[i]  |\n", k);
    printf("|----|-------|-----------|---------------|------------|\n");
    for (int i = 0; i < n; ++i)
    {
        printf("| %2d | ", i);
        printf(" %3d  | ", SA[i]);
        printf(" %5d    | ", getRA(SA[i]));
        printf(" %7d      | ", getRA(SA[i] + k));
        printf("   %4d    |\n", tempRA[SA[i]]);
    }
    puts("========================================================");
}

void TEST()
{
    test = true;
    int values[] = {10000000, 20000000, 298769211};

    int siz = sizeof(values) / sizeof(int);
    double avg_cpi = 0;

    puts("");
    puts("|         n | Runtime(s) |    TPI(ms)   |");
    puts("|----------:|:----------:|:------------:|");
    const int SIZ = values[siz - 1] + 1;
    T = new char[SIZ + 1](0);
    SA = new int[SIZ](0);
    tempSA = new int[SIZ](0);
    RA = new int[SIZ](0);
    tempRA = new int[SIZ](0);
    L = new int[SIZ](0);

    for (int k = 0; k < siz; ++k)
    {
        int n = values[k];
        for (int i = 0; i < n; ++i)
        {
            if (rand() & 1)
            {
                T[i] = 'A' + (rand() % 26);
            }
            else if (rand() & 1)
            {
                T[i] = 'a' + (rand() % 26);
            }
            else
            {
                T[i] = '0' + (rand() % 10);
            }
        }
        T[n] = 0;

        time_t start = clock();
        buildSA(); // builds the suffix array
        time_t stop = clock();

        double time = (double)(stop - start) / CLOCKS_PER_SEC;
        double cpi = (double)(stop - start) / (n * log2(n));
        printf("| `%7d` |   `%5.3f`  | `%0.8f` |\n", n, time, cpi);

        if (k)
            avg_cpi += (values[k] - values[k - 1]) * cpi;
        else
            avg_cpi += values[k] * cpi;
    }
    avg_cpi /= values[siz - 1];

    printf("\n");
    printf("**Average *Time Per Instructions*** = `%.10f ms`\n", avg_cpi);
}

int main()
{
    //RUN();
    TEST();
    return 0;
}

#endif
