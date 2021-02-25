# USB_Host_Shield_Library_2.0_BTXBOX
The code is released under the GNU General Public License.

## Summary
This library is based on the USB Host Shield Library 2.0 with modifications to support Secure Simple Pairing on Bluetooth to use the Microsoft Game Controller (for Xbox One S/Windows MODEL 1708).

## 概要
このライブラリは、USB Host Shield Library 2.0 を元にして、Microsoftゲームコントローラー（Xbox One S/Windows用 MODEL 1708）が繋がるように、ライブラリのBluetoothの部分を Secure Simple Pairing 対応に改造したものです。
USB_Host_Shield_2.0については下記を参照してください。

USB_Host_Shield_2.0 リポジトリ  
<http://github.com/felis/USB_Host_Shield_2.0>  
プロジェクトサイト   
<https://chome.nerpa.tech/arduino_usb_host_shield_projects/>

元のライブラリがSecure Simple Pairing(SSP)に対応する前に、自分で改造したライブラリです。今では、元のライブラリも SSP に対応しています。

ここでは、その自分で改造したライブラリを残しておいて、なおかつ自分の勉強と練習のために、さらに好き勝手に手を加えています。
今回(2021/03/00)、元のライブラリや前の自作ライブラリを書き変えまくって、かなり大きな変更を加えたので、互換性等はあまりないです。

動作保証はしません。ユーザーサポートもしません。使用は自己責任で。何か問題があっても自分で何とかしてください。

多分、色々と間違ってると思います。

オープンソースなので、ライセンスの範囲内で自由に使ってやってください。

USB_Host_Shield_Library_2.0_BTXBOX リポジトリ  
<https://github.com/HisashiKato/USB_Host_Shield_Library_2.0_BTXBOX>  

　  

## ハードウェア
* Arduino 及び、その互換機  
* USB Host Shield　2.0  
* Bluetooth 4.0 USBアダプタ

   
   

## 使用方法
### Arduino IDE にライブラリを組み込む  
USB Host Library Rev.2.0 BTXBOXは、Arduino IDEで使用できます。
    
1. Code の Download ZIP 、または下記URLから、ライブラリのZIPをダウンロードします。  
<https://github.com/HisashiKato/USB_Host_Shield_Library_2.0_BTXBOX/archive/master.zip>  
   
2. Arduino IDE を起動して、スケッチ > ライブラリをインクルード > .ZIP形式のライブラリをインストール で、ダウンロードした USB_Host_Shield_Library_2.0_BTXBOX-master.zip を指定します。  
   
3. Arduino IDE に、「ライブラリが追加されました。「ライブラリをインクルード」メニューを確認してください。」と表示されれば成功。

4. Arduino IDE で、メニューの スケッチ > ライブラリをインクルード のライブラリの一覧に、USB Host Shield Library 2.0 BTXBOX が表示されているのを確認してください。

   
   
   
### ライブラリの使用方法
先ずは、元の USB Host Library Rev.2.0 のライブラリの使用方法を読んでください。

基本的な使い方は、本家の USB Host Library Rev.2.0 の BTHID 等とほぼ同じです。但し、BTD の代わりに、このライブラリの BTDSSP を使います。 そして、ペアリングに PINコードを使用しません。

ペアリングモードが XB1SBTR Xb1s(&Btdssp, PAIR);

ペアリングが済んだ後は XB1SBTR Xb1s(&Btdssp);

となっています。スケッチ例を参照してください。

このライブラリでは、Arduino の内部のフラッシュ領域にデータを書き込み読み出すEEPROMライブラリを使用しています。注意してください。具体的にはペアリングに成功したデバイスのBDアドレス(Bluetooth Device Address) 6byte、それに続けてペアリング時に生成されたリンクキー 16byte を、アドレスの0番地から書き込んで、再接続時に読みに行っています。（このフラッシュ領域のデータは、スケッチを書き変えても変更されません）

SSP のリンクキーを保存するので、再接続等が確実になっています。

このライブラリは、BluetoothのSSP接続にだけ対応しています。従来のPS3コントローラー,PS4コントローラー,Wiiリモコン等との接続や、旧来のPINコードを用いた接続は外しました。USB有線での接続にも対応していません。それらは元の USB Host Library Rev.2.0 ライブラリを使用してください。

   
   

## 対応コントローラー

### Microsoftゲームコントローラー（Xbox One S/Windows用 MODEL 1708）

コントローラーの基本の情報の受信に加えて、振動用データの送信に対応しました。

ゲームコントローラーのファームウェアが違うと、送られてくるデータが異なることがあります。動作確認を行っているコントローラーのファームウェアは、ライブラリ製作時の最新の 4.8.1923.0 です。使用するコントローラーのファームウェアの確認と更新をお願いします。

### Nintendo Switch Pro コントローラー

暫定版です。コントローラーの基本の情報の受信のみです。初期化や振動やジャイロ等には対応していません（サブコマンドを入れていません）

## 参考にさせて頂いたプロジェクトやWEBサイト
<https://github.com/felis/USB_Host_Shield_2.0>：USB_Host_Shield_2.0 の本家

<https://www.silex.jp/blog/wireless/>：サイレックス・テクノロジー株式会社 のblog、Wireless・のおと（Bluetoothの基本）

<http://www.yts.rdy.jp/pic/GB002/GB002.html>：YTSさんの、YTSのホームページ（SSPの詳細等）

<https://hackaday.io/project/170365-blueretro>：Jacques Gagnon 氏の"BlueRetro"（とても参考になりました！！！）

<https://github.com/atar-axis/xpadneo>：Advanced Linux Driver for Xbox One Wireless Gamepad

<https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering>：Nintendo_Switch_Reverse_Engineering

その他、思いついたら追加します。
こうして作ることが出来たのも、みなさんのおかげです。
