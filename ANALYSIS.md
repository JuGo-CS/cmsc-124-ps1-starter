## Question 1: Language Trade-offs

#### 1. Hash Maps (Python dictionary vs. Implemented C Map)
- Python dictionaries implement dynamic insertion, automatic resizing, built-in ordering, and key-value lookups. However, these come with some trade-offs. In terms of memory, each entry wraps the data in a full `PyObject` header (which includes storing reference counts and type info) and has extra lookup table overhead. This results in using more RAM per entry compared to a created C struct pointer. Accessing the values requires chasing pointer indirection and constantly updating the reference counts, which will slow down the tight loops.
- The difference will be very noticeable when storing millions of small entries. Python's extra memory degrades the CPU cache locality and causes performance bottlenecks.

#### 2. Dynamic Typing (JavaScript vs. Implemented C Tagged Union)
- In JavaScript, variables can change types freely. Again, these come with costs. In memory, engines like the V8 attach extra memory overhead to track the types of these variables dynamically at runtime. With speed, operations need continuous runtime type checks.
- There can be high-throughput or real-life applications where unexpected JIT deoptimizations can cause sudden frame drops and latency spikes.

#### 3. Managed Strings (Java String vs. Implemented C char *)
- Java has automatic memory management (garbage collection), built-in length tracking, bounds checking, string immutability, and safe concatenation without manual allocation, but includes some trade-offs. Every Java string is an object with 12 to 16 bytes, a 4-byte array reference, and includes character payload alignment padding, which takes about 24 to 32 bytes in total. In comparison, storing "cat" in C takes 4 bytes. Also, with garbage collection overhead, creating temporary strings will allocate short-lived objects on the heap, triggering periodic garbage collection pauses to clean them up.
- This is noticeable in high-frequency network services or embedded microcontrollers, where unpredictable garbage collection pauses or memory bloat can cause severe latency.



## Question 2: Manual vs. Compiler-Enforced Tag Checks
- The C version offers more flexibility but also less safety, by letting you implement it yourself. It allows the person implementing the tagged union more freedom over what happens if the tag doesn't match, leave some variants unchecked, or maybe even skip the tag check entirely in favor of performance (though at the detriment of safety). You could even define innovative or unconventional behavior on a mismatch, which is very much appreciated by people trying to implement more specialized languages. Perhaps, one could implement a language where passing in a value like "123" into dt_value_as_int isn't immediately rejected, but rather parsed into an integer. There is definitely merit to this granular-control approach, however it of course has the drawback of making the implementer responsible for maintaining every invariant.



## Question 3: Dropping Insertion Order in dt_map
- Dropping the insertion-order tracking on the dt_map implementation would simplify the architecture. The data structure functions as a traditional hash map and relies only on single-linked bucket chains for the key-value pair store operations. Removing insertion-order tracking will remove deterministic iteration from the data structure, thus causing two main breakdowns:
    - Non-Deterministic iteration: dt_map_key_at will no longer retrieve keys in the exact order they were inserted. The key traversal will solely depend on the hash bucket.
    - Inconsistent display: printing or converting the map to text won't have a reliable order anymore since the key positions will change based on how the hash buckets fill up.
- When shipping it, it would be entirely based on whether insertion order actually matters for the application. If printing or retrieving the keys in the exact order they were added is needed, then obviously I would not ship it. On the other hand, if the map is used only for background lookups where the key order is irrelevant, I would remove the extra pointers (dropping the insertion order) because doing so cuts pointer overhead per node in half, which will reduce memory usage and simplify the `put` and `remove` logic.



## Question 4: Access-After-Release vs. Unreleased Allocations
- In a long-running server, accessing after release could possibly have catastrophic consequences like corrupting data, or crashing the server entirely. A memory leak can also be detrimental, but for a different reason: small memory leaks can add up over thousands to millions of requests, causing the server to slowly exhaust its memory, driving up costs. For a command-line tool though, an access after release would still possibly crash/corrupt the tool, potentially losing progress or input, but your machine will reset everything after the crash either way. A memory leak is even less significant, sometimes even going unaddressed for small scale/less memory-intensive programs, due to how much memory is available anyways.

