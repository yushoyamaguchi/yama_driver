# memory-mapped-IO領域とのやり取り
writelを使う

# PCIへの登録
pci_register_driver() ？

# NetDeviceとしての登録
register_netdev()？

# 割り込み
どうしたらいい？？

# 実装すべきことがら
- PCI登録
- netdev登録
- 中の初期化処理
- 中の送受信処理
- 割り込み関連

# nicとこのドライバを結びつけるのは何？
xv6のときはmain関数からe1000_init関数を呼び出して、その中でPCIFuncEnableとかを呼び出した。
<bar>
また送信時はe1000_tx関数を直接呼び出し、受信時はe1000_rxを呼び出すハンドラ関数がirq11番の割り込みの時に呼び出されるように直接設定した。
<bar>
Linuxではどうする？？

# まずやること
はじめにPCIが登録されたときに実行される関数を作って、PCIへの登録とnetdevへの登録をする。