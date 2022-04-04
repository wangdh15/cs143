(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)
class List {

   item : String;
   next : List;
   init(s : String, n : List) : List {
      {
         item <- s;
         next <- n;
         self;
      }
   };

   next() : List {
      next
   };

   item() : String {
      item
   };
};

class Stack {
   top: List;

   push(s : String) : Stack {
      {
         top <- (new List).init(s, top);
         self;
      }
   };

   pop() : Stack {
      let
          nil : List
      in
            if (isvoid top) then
               self
            else
            {
               top <- top.next();
               self;
            }
            fi
   };

   top() : List {
      top
   };
};

class DCommand {

   io : IO <- new IO;
   eval(s : Stack) : Object {
      let
         cur : List <- s.top()
      in
         {
            while (not isvoid cur) loop {
               io.out_string(cur.item().concat("\n"));
               cur <- cur.next();
            }
            pool;
            self;
         }
   };
};

class ECommand {
   item1 : String;
   item2 : String;
   item3 : String;
   list: List;
   cvt : A2I <- new A2I;

   eval(s : Stack) : Object {
      {
         list <- s.top();
         if (isvoid list) then
            self
         else
            {
               item1 <- s.top().item();
               if (item1 = "+") then
                  {
                     s.pop();
                     item2 <- s.top().item();
                     s.pop();
                     item3 <- s.top().item();
                     s.pop();
                     s.push(cvt.i2a(cvt.a2i(item2) + cvt.a2i(item3)));
                  }
               else
                  if (item1 = "s") then
                     {
                        s.pop();
                        item2 <- s.top().item();
                        s.pop();
                        item3 <- s.top().item();
                        s.pop();
                        s.push(item2);
                        s.push(item3);
                     }
                  else
                     self
                  fi
               fi;
            }
         fi;
      }
   };
};

class Main inherits IO {

   cvr : A2I;
   stk : Stack <- new Stack;
   flag : Bool;
   cmd: String;

   main() : Object {
      {
         flag <- true;
         while (flag) loop {
            out_string(">");
            cmd <- in_string();
            if (cmd = "x") then
               flag <- false
            else
               if (cmd = "d") then
                  (new DCommand).eval(stk)
               else
                  if (cmd = "e") then
                     (new ECommand).eval(stk)
                  else
                     stk.push(cmd)
                  fi
               fi
            fi;
         }
         pool;
      }
   };
};
