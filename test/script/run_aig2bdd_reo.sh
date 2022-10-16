for f in `cat mcnc-comb_small.txt`
do
    aig2bdd_reo $f
done
