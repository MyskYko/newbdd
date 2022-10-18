if [ -z $1 ] || [ -z $2 ]
then
    echo "Specify list and directory"
    exit 1    
fi

dc2="dc2; dc2; dc2; dc2; dc2"
opt1="$dc2; compress2rs; $dc2; resyn; $dc2; resyn2; $dc2; resyn3; $dc2; resub –l -N 2 -K 16; $dc2; iresyn –l; $dc2; &get; &fraig –x -C 1000; &put; $dc2"
opt2="dch; if -K 4 -a -m; mfs2 -e; st; compress2rs"

for h in `cat $1`
do
    echo -n "$h "
    g=$2/`basename $h`
    cp $h $g
    f=$g.tmp.aig
    cp $g $f
    n=`abc -q "read $f; ps" | grep -Po "nd =[ ]*\K[0-9]*"`
    #echo $n
    s=0
    while true
    do
        m=`abc -q "read $f; ps" | grep -Po "nd =[ ]*\K[0-9]*"`
        abc -q "read $f; $opt1; write $f"
        t=`abc -q "read $f; ps" | grep -Po "nd =[ ]*\K[0-9]*"`
        while (($m > $t))
        do
            m=$t
            abc -q "read $f; $opt1; write $f"
            t=`abc -q "read $f; ps" | grep -Po "nd =[ ]*\K[0-9]*"`            
        done
        #echo $m
        if (($n > $m))
        then
            n=$m
            cp $g $f
            s=0
        else
            s=$((s+1))
            if (($s > 10))
            then
                break
            fi
        fi
        abc -q "read $f; $opt2; write $f"
    done
    echo $n
done
