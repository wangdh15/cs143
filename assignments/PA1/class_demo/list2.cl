class List inherits A2I {
    item: Object;
    next: List;

    init(i: Object, n: List) : List {
        {
            item <- i;
            next <- n;
            self;
        }
    };

    flatten() : String {
        let string: String <-
            -- 根据item的类型来得到对应的字符串
            case item of
                i: Int => i2a(i);
                s: String => s;
                o: Object => { abort(); "";};
            esac
        in
        -- 前面定义完变量，下面进行运算
            if (isvoid next) then
                string
            else
                string.concat(next.flatten())
            fi
    };
};


class Main inherits IO {
    main() : Object {
        let
            -- 定义一堆变量
            hello : String <- "hello ",
            world: String <- "world! ",
            i : Int <- 42,
            newline: String <- "\n",
            nil : List,  -- 默认nil为空
            list: List <- -- 开始构造list
                (new List).init(hello,
                    (new List).init(world,
                        (new List).init(i,
                            (new List).init(newline, nil))))
        in
            -- 执行
            out_string(list.flatten())
    };
};
