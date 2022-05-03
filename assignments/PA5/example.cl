
(*  Example cool program testing as many aspects of the code generator
    as possible.
 *)

class A {

  add(p: Int): Int {
    1
  };

  sub(p: Int): Int {
    3
  };
};


class B inherits A {
  add(p: Int) : Int {
    2
  };
};

class C {
  p : A <- new B;

  func(): Object {
    p.add(1)
  };
};

class Main {
  a : Int <- 1;
  b : Int <- 2;
  c : Int <- a + b;

  add(p1:Int, p2: Int, p3: Int): Int {
    a + p1 + p2 + p3
  };

  test(p: Int): Int {
    if p < 1 then
      p
    else
      1
    fi
  };

  test2(): Object {
    case 1 of
     e1: Int => 1;
     e2: Object => e2;
    esac
  };

  main():Object {
      add(a, b, c)
    };

};

