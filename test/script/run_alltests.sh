./setup_oldbdd.sh
set -x
./run_aig2bdd_test.sh mcnc-comb_small.txt
./run_aig2bdd_gctest.sh mcnc-comb_small.txt
./run_aig2bdd_reotest.sh mcnc-comb_small.txt
./run_aig2bdd_reotest2.sh mcnc-comb_small.txt
./run_tra_test.sh mcnc-pla_small.txt
