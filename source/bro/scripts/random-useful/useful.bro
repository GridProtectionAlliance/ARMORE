function range(lower: int, upper: int):
        vector of int {
                if (lower >= upper) {
                        return vector();
                }
                else {
                        local v: vector of int = vector(lower);
                        local i: count = 1;
                        local r: vector of int = range(lower + 1, upper);
                        for (n in r) {
                                v[i] = r[n];
                                ++i;
                        }
                        return v;
                }
        }
