# libdo

libdo is an embeddable library for [predicate-driven programming](http://shoaib-ahmed.com/2018/predicate-driven-programming) in C and C++. 

---

### Features

* C89 compatible
* Simple API
* Priority based dispatch
* Expirable handlers
* No restrictions on adding/removing handlers from within handlers
* Test suites

---

### Installation

```sh
git clone https://github.com/Sufi-Al-Hussaini/libdo.git
```

Copy `libdo.{h,c}` and `vector.h` to your source code tree and add `libdo.c` to your build system source files list.

Run `make` to compile C & C++ tests and `./tests` or `./testscpp` to run them.

#### For Arduino

* Create a directory named `libdo` in your `~/Arduino/libraries` directory. 
* Copy `libdo.{h,c}` and `vector.h` into it.
* Import the `libdo` library as usual. Typically, `Sketch -> Include Library -> libdo` in the Arduino IDE. 

---

### Usage

**Note:** Error-checking removed for brevity.
```c
/* Create a doer */
struct do_doer *doer = do_init();

/* Create a work with a work function and predicate */
bool run_work = true;
struct do_work *work = do_work_if(work_function, NULL, &run_work);

/* Ask the doer to do it */
/* This also gives up ownership of the work instance to the doer */
do_so(doer, work);

/* Cleanup */
do_destroy(doer);
```
---

### Documentation

Documentation and possible applications can be found on this [blog post](http://shoaib-ahmed.com/2018/predicate-driven-programming).

---

### Best practices

* Don't sleep in your predicate or work functions. Instead, add a `work` with a time predicate, if you want to do something after a delay.
* Add a delay between subsequent `loop()` calls, to keep the processor happy. Ideally, have a blocking function call before the `loop()` call.
* Make sure you remove `works` that you don't need.

---

### Todo

Work for these features is planned and under way:

* Code documentation
* C++11 header-only wrapper class
* Examples

---

### Contributing 

* Submit pull requests for bug-fixes directly. 
* For everything else (feature request, feature addition, discussions, etc.) open an issue first, so that it is easy for everyone to track.
* Follow libdo's code style and write appropriate test.

---

### License

The MIT License (MIT)
