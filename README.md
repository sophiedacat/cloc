# cloc

A line counter. In C++. 

I wrote this because I kept needing to count lines in projects, and I didn’t want to install Perl or Python just for that. The original `cloc` is great, but I wanted something I can compile once, drop anywhere, and run in a split second.

---

**What it handles:**

- C++, Python, JSON, XML, Markdown, and `.sln` files.
- `//`, `/* */`, and `#` comments – including multi-line blocks.
- Recursive folders. Point it at a directory and it walks through everything.

**Build it:**

```bash
g++ -std=c++17 -O2 cloc.cpp -o cloc
```

**Output example:**

```
cloc | T=0.12 s (316.7 files/s, 2483.3 lines/s)
----------------------------------------------
Language    files    blank    comment    code
C++            25       87        142    1823
Python          8       23         45     312
----------------------------------------------
SUM:           38      122        187    2291
```

**Why this exists:**
- I wanted something I can install and use anywhere
- Old version was slow and required dependencies
