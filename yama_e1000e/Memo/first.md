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

# まずやること
はじめにPCIが登録されたときに実行される関数を作って、PCIへの登録とnetdevへの登録をする。