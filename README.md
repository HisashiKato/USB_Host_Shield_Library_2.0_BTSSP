# USB_Host_Shield_Library_2.0_BTXBOX
The code is released under the GNU General Public License.

## 概要
マイクロソフト ゲームコントローラー Bluetooth/有線接続 Xbox One/Windows対応 MODEL 1708 ( Microsoft Game Controller Xbox One S / Windows MODEL 1708 )を、USB Bluetoothアダプタ経由でArduinoに接続するために、USB Host Library Rev.2.0の Bluetooth HID 部分を、SSP (Simple Secure Pairing) Just Works を用いたペアリングに変更しました。USB_Host_Shield_2.0リポジトリからフォークしようとしたのですが、素人故の勉強不足で出来なかったので、とりあえず新規リポジトリで公開します。 USB_Host_Shield_2.0については下記を参照してください。

USB_Host_Shield_2.0 リポジトリ  
<http://github.com/felis/USB_Host_Shield_2.0>  
プロジェクトサイト   
<https://chome.nerpa.tech/arduino_usb_host_shield_projects/>

Bluettoth USBアダプタを刺した USB Host Shield 2.0 と、USB Host Library Rev.2.0 を使って、マイクロソフトのゲームコントローラー (Xbox One S から使われ始めた MODEL 1708)を Bluetooth で Arduino に接続しようと試みたのですが、~~ゲームコントローラーが4ケタのPINコードを使用する古いペアリングには対応していないので出来ませんでした。そこで USB Host Library Rev.2.0 の BTD.cpp を、Bluetooth Classic SSP でペアリング出来るように改造してみました。従来のPS3コントローラー,PS4コントローラー,Wiiリモコン等との Bluetooth接続の機能は外しました~~(追記：2020/11/16)接続に対応したっぽいです。  
USB Host Library Rev.2.0 の、過去の BTD.cpp を、Bluetooth Classic SSP でペアリング出来るように改造したものです。従来のPS3コントローラー,PS4コントローラー,Wiiリモコン等との Bluetooth接続の機能は外しました。そのため、若干サイズが小さくなっています。SSP接続専用のライブラリとして、ここに残しておきます。

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
  
2. ダウンロードした　USB_Host_Shield_Library_2.0_BTXBOX-master.zip　を　USB_Host_Shield_Library_2.0_BTXBOX.zip にリネームします。（必須ではないです）  
  
3. Arduino IDE を起動して、スケッチ > ライブラリをインクルード > .ZIP形式のライブラリをインストール で、ダウンロードした USB_Host_Shield_Library_2.0_BTXBOX.zip を指定します。  
   
4. Arduino IDE に、「ライブラリが追加されました。「ライブラリをインクルード」メニューを確認してください。」と表示されれば成功。

5. Arduino IDE で、メニューの スケッチ > ライブラリをインクルード のライブラリの一覧に、USB Host Shield Library 2.0 BTXBOX が表示されているのを確認してください。

### ライブラリの使用方法  
基本的な使い方は、本家の USB Host Library Rev.2.0 の BTHID とほぼ同じです。但し、BTD の代わりに、このライブラリの BTDSSP を使います。
そして、ペアリングに PINコードを使用しないので、

ペアリングモードが
`BTXBOX btxbox(&Btdssp, PAIR);`

ペアリングが済んだ後は
`BTXBOX btxbox(&Btdssp);`

となります。スケッチ例を参照してください。  


このライブラリでは、Arduino の内部のフラッシュ領域にデータを書き込み読み出すEEPROMライブラリを使用しています。注意してください。具体的にはペアリングに成功したデバイスのBDアドレス(Bluetooth Device Address) 6byte、それに続けてペアリング時に生成されたリンクキー 16byte を、アドレスの0番地から書き込んで、再接続時に読みに行っています。（このフラッシュ領域のデータはスケッチの書き込みの影響を受けません）

ゲームコントローラーのファームウェアが違うと、送られてくるデータが異なることがあります。今回、動作確認を行っているコントローラーのファームウェアは、ライブラリ製作時の最新の 4.8.1923.0 です。使用するコントローラーのファームウェアの確認と更新をお願いします。

プログラムのソースコードを見ると分かると思いますが、素人改造のライブラリです。それでも、もしよければ、使ってやってください。
