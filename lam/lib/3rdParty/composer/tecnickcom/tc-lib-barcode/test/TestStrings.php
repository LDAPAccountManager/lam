<?php

// @codingStandardsIgnoreFile
/**
 * TestStrings.php
 *
 * @since       2016-08-31
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test;

/**
 * Barcode class test
 *
 * @since       2016-08-31
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class TestStrings
{
    /**
     * Array containing testing strings.
     *
     * @var array<array{string}>
     *
     * @phpcs:disable Generic.Files.LineLength.TooLong
     */
    public static array $data = [
        // Reserved keywords
        [
            '__halt_compiler abstract and array as break callable case catch class clone const continue declare default die do echo else elseif empty enddeclare endfor endforeach endif endswitch endwhile eval exit extends final for foreach function global goto if implements include include_once instanceof insteadof interface isset list namespace new or print private protected public require require_once return static switch throw trait try unset use var while xor',
        ],
        [
            '__CLASS__ __DIR__ __FILE__ __FUNCTION__ __LINE__ __METHOD__ __NAMESPACE__ __TRAIT__ boolean bool integer float double string array object resource undefined undef null NULL (null) nil NIL true false True False TRUE FALSE None hasOwnProperty',
        ],
        // Numeric Strings
        [
            '0 1 2 3 5 7 97 397 997 7919 99991 104729 01 012 0123 01234 012345 0123456 01234567 012345678 0123456789 1234567890 12 123 1234 12345 123456 1234567 12345678 123456789 1234567890 012345678900112233445566778899 012345678900112233445566778899000111222333444555666777888999 123456789012345678901234567890123456789 999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999',
        ],
        [
            '1.00 $1.00 1/2 1E2 1E02 1E+02 -1 -1.00 -$1.00 -1/2 -1E2 -1E02 -1E+02 1/0 0/0 -2147483648/-1 -9223372036854775808/-1 0.00 0..0 . 0.0.0 0,00 0,,0 , 0,0,0 0.0/0 1.0/0.0 0.0/0.0 1,0/0,0 0,0/0,0 --1 - -. -, NaN Infinity -Infinity INF 1#INF -1#IND 1#QNAN 1#SNAN 1#IND 0x0 0xffffffff 0xffffffffffffffff 0xabad1dea 1,000.00 1 000.00 1\'000.00 1,000,000.00 1 000 000.00 1\'000\'000.00 1.000,00 1 000,00 1\'000,00 1.000.000,00 1 000 000,00 1\'000\'000,00 01000 08 09 2.2250738585072011e-308 012-345-678-901-234-567-890-123-456-789',
        ],
        // Special Characters
        [
            ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~',
        ],
        [
            'Roses are [0;31mred[0m, violets are [0;34mblue. Hope you enjoy terminal hue But now...[20Cfor my greatest trick...[8m The quick brown fox...',
        ],
        [
            '<<< ((( [[[ {{{ """ \'\'\' ``` ### ~~~ @@@ £££ $$$ %%% ^^^ &&& *** --- +++ === ___ ::: ;;; ,,, ... ??? ¬¬¬ ||| /// \\\\\\ !!! }}} ]]] ))) >>> ./;\'[]\-= <>?:"{}|_+ !@#$%^&*()`~ "\'"\'"\'\'\'\'"',
        ],
        // Unicode Symbols
        [
            'Ω≈ç√∫˜µ≤≥÷ åß∂ƒ©˙∆˚¬…æ œ∑´®†¥¨ˆøπ“‘ ¡™£¢∞§¶•ªº–≠ ¸˛Ç◊ı˜Â¯˘¿ ÅÍÎÏ˝ÓÔÒÚÆ☃ Œ„´‰ˇÁ¨ˆØ∏”’ `⁄€‹›ﬁﬂ‡°·‚—± ⅛⅜⅝⅞ ЁЂЃЄЅІЇЈЉЊЋЌЍЎЏАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя ٠١٢٣٤٥٦٧٨٩   ᠎ 　 ﻿ ␣ ␢ ␡ ⁰⁴⁵ ₀₁₂ ⁰⁴⁵₀₁₂',
        ],
        // Two-Byte Characters
        [
            '田中さんにあげて下さい パーティーへ行かないか 和製漢語 部落格 사회과학원 어학연구소 찦차를 타고 온 펲시맨과 쑛다리 똠방각하 社會科學院語學研究所 울란바토르 𠜎𠜱𠝹𠱓𠱸𠲖𠳏',
        ],
        // Japanese Emoticons
        [
            'ヽ༼ຈل͜ຈ༽ﾉ ヽ༼ຈل͜ຈ༽ﾉ  (｡◕ ∀ ◕｡) ｀ｨ(´∀｀∩ __ﾛ(,_,*) ・(￣∀￣)・:*: ﾟ･✿ヾ╲(｡◕‿◕｡)╱✿･ﾟ ,。・:*:・゜’( ☻ ω ☻ )。・:*:・゜’ (╯°□°）╯︵ ┻━┻)   (ﾉಥ益ಥ）ﾉ﻿ ┻━┻ ( ͡° ͜ʖ ͡°)',
        ],
        // Emoji
        [
            '😍 👩🏽 👾 🙇 💁 🙅 🙆 🙋 🙎 🙍  🐵 🙈 🙉 🙊 ❤️ 💔 💌 💕 💞 💓 💗 💖 💘 💝 💟 💜 💛 💚 💙 ✋🏿 💪🏿 👐🏿 🙌🏿 👏🏿 🙏🏿 🚾 🆒 🆓 🆕 🆖 🆗 🆙 🏧 0️⃣ 1️⃣ 2️⃣ 3️⃣ 4️⃣ 5️⃣ 6️⃣ 7️⃣ 8️⃣ 9️⃣ 🔟',
        ],
        // Regional Indicator Symbols
        ['🇺🇸🇷🇺🇸 🇦🇫🇦🇲🇸  🇺🇸🇷🇺🇸🇦🇫🇦🇲 🇺🇸🇷🇺🇸🇦'],
        // Unicode Numbers
        ['１２３ ١٢٣'],
        // Right-To-Left Strings
        [
            'ثم نفس سقطت وبالتحديد،, جزيرتي باستخدام أن دنو. إذ هنا؟ الستار وتنصيب كان. أهّل ايطاليا، بريطانيا-فرنسا قد أخذ. سليمان، إتفاقية بين ما, يذكر الحدود أي بعد, معاملة بولندا، الإطلاق عل إيو. בְּרֵאשִׁית, בָּרָא אֱלֹהִים, אֵת הַשָּׁמַיִם, וְאֵת הָאָרֶץ הָיְתָהtestالصفحات التّحول ﷽ ﷺ مُنَاقَشَةُ سُبُلِ اِسْتِخْدَامِ اللُّغَةِ فِي النُّظُمِ الْقَائِمَةِ وَفِيم يَخُصَّ التَّطْبِيقَاتُ الْحاسُوبِيَّةُ، ',
        ],
        // Trick Unicode
        ['‪‪test‪ ‫test‫  test  test⁠test‫ ⁦test⁧'],
        // Strings which contain "corrupted" text
        ['Ṱ̺̺̕o͞ ̷i̲̬͇̪͙n̝̗͕v̟̜̘̦͟o̶̙̰̠kè͚̮̺̪̹̱̤ ̖t̝͕̳̣̻̪͞h̼͓̲̦̳̘̲e͇̣̰̦̬͎ ̢̼̻̱̘h͚͎͙̜̣̲ͅi̦̲̣̰̤v̻͍e̺̭̳̪̰-m̢iͅn̖̺̞̲̯̰d̵̼̟͙̩̼̘̳ ̞̥̱̳̭r̛̗̘e͙p͠r̼̞̻̭̗e̺̠̣͟s̘͇̳͍̝͉e͉̥̯̞̲͚̬͜ǹ̬͎͎̟̖͇̤t͍̬̤͓̼̭͘ͅi̪̱n͠g̴͉ ͏͉ͅc̬̟h͡a̫̻̯͘o̫̟̖͍̙̝͉s̗̦̲.̨̹͈̣ ̡͓̞ͅI̗̘̦͝n͇͇͙v̮̫ok̲̫̙͈i̖͙̭̹̠̞n̡̻̮̣̺g̲͈͙̭͙̬͎ ̰t͔̦h̞̲e̢̤ ͍̬̲͖f̴̘͕̣è͖ẹ̥̩l͖͔͚i͓͚̦͠n͖͍̗͓̳̮g͍ ̨o͚̪͡f̘̣̬ ̖̘͖̟͙̮c҉͔̫͖͓͇͖ͅh̵̤̣͚͔á̗̼͕ͅo̼̣̥s̱͈̺̖̦̻͢.̛̖̞̠̫̰ ̗̺͖̹̯͓'],
        ['Ṯ̤͍̥͇͈h̲́e͏͓̼̗̙̼̣͔ ͇̜̱̠͓͍ͅN͕͠e̗̱z̘̝̜̺͙p̤̺̹͍̯͚e̠̻̠͜r̨̤͍̺̖͔̖̖d̠̟̭̬̝͟i̦͖̩͓͔̤a̠̗̬͉̙n͚͜ ̻̞̰͚ͅh̵͉i̳̞v̢͇ḙ͎͟-҉̭̩̼͔m̤̭̫i͕͇̝̦n̗͙ḍ̟ ̯̲͕͞ǫ̟̯̰̲͙̻̝f ̪̰̰̗̖̭̘͘c̦͍̲̞͍̩̙ḥ͚a̮͎̟̙͜ơ̩̹͎s̤.̝̝ ҉Z̡̖̜͖̰̣͉̜a͖̰͙̬͡l̲̫̳͍̩g̡̟̼̱͚̞̬ͅo̗͜.̟ ̦H̬̤̗̤͝e͜ ̜̥̝̻͍̟́w̕h̖̯͓o̝͙̖͎̱̮ ҉̺̙̞̟͈W̷̼̭a̺̪͍į͈͕̭͙̯̜t̶̼̮s̘͙͖̕ ̠̫̠B̻͍͙͉̳ͅe̵h̵̬͇̫͙i̹͓̳̳̮͎̫̕n͟d̴̪̜̖ ̰͉̩͇͙̲͞ͅT͖̼͓̪͢h͏͓̮̻e̬̝̟ͅ ̤̹̝W͙̞̝͔͇͝ͅa͏͓͔̹̼̣l̴͔̰̤̟͔ḽ̫.͕'],
        // Unicode Upsidedown
        [
            '˙ɐnbᴉlɐ ɐuƃɐɯ ǝɹolop ʇǝ ǝɹoqɐl ʇn ʇunpᴉpᴉɔuᴉ ɹodɯǝʇ poɯsnᴉǝ op pǝs \'ʇᴉlǝ ƃuᴉɔsᴉdᴉpɐ ɹnʇǝʇɔǝsuoɔ \'ʇǝɯɐ ʇᴉs ɹolop ɯnsdᴉ ɯǝɹo˥ 00˙Ɩ$-',
        ],
        // Unicode font
        [
            'Ｔｈｅ ｑｕｉｃｋ ｂｒｏｗｎ ｆｏｘ ｊｕｍｐｓ ｏｖｅｒ ｔｈｅ ｌａｚｙ ｄｏｇ 𝐓𝐡𝐞 𝐪𝐮𝐢𝐜𝐤 𝐛𝐫𝐨𝐰𝐧 𝐟𝐨𝐱 𝐣𝐮𝐦𝐩𝐬 𝐨𝐯𝐞𝐫 𝐭𝐡𝐞 𝐥𝐚𝐳𝐲 𝐝𝐨𝐠 𝕿𝖍𝖊 𝖖𝖚𝖎𝖈𝖐 𝖇𝖗𝖔𝖜𝖓 𝖋𝖔𝖝 𝖏𝖚𝖒𝖕𝖘 𝖔𝖛𝖊𝖗 𝖙𝖍𝖊 𝖑𝖆𝖟𝖞 𝖉𝖔𝖌 𝑻𝒉𝒆 𝒒𝒖𝒊𝒄𝒌 𝒃𝒓𝒐𝒘𝒏 𝒇𝒐𝒙 𝒋𝒖𝒎𝒑𝒔 𝒐𝒗𝒆𝒓 𝒕𝒉𝒆 𝒍𝒂𝒛𝒚 𝒅𝒐𝒈 𝓣𝓱𝓮 𝓺𝓾𝓲𝓬𝓴 𝓫𝓻𝓸𝔀𝓷 𝓯𝓸𝔁 𝓳𝓾𝓶𝓹𝓼 𝓸𝓿𝓮𝓻 𝓽𝓱𝓮 𝓵𝓪𝔃𝔂 𝓭𝓸𝓰 ',
        ],
        [
            '𝕋𝕙𝕖 𝕢𝕦𝕚𝕔𝕜 𝕓𝕣𝕠𝕨𝕟 𝕗𝕠𝕩 𝕛𝕦𝕞𝕡𝕤 𝕠𝕧𝕖𝕣 𝕥𝕙𝕖 𝕝𝕒𝕫𝕪 𝕕𝕠𝕘 𝚃𝚑𝚎 𝚚𝚞𝚒𝚌𝚔 𝚋𝚛𝚘𝚠𝚗 𝚏𝚘𝚡 𝚓𝚞𝚖𝚙𝚜 𝚘𝚟𝚎𝚛 𝚝𝚑𝚎 𝚕𝚊𝚣𝚢 𝚍𝚘𝚐 ⒯⒣⒠ ⒬⒰⒤⒞⒦ ⒝⒭⒪⒲⒩ ⒡⒪⒳ ⒥⒰⒨⒫⒮ ⒪⒱⒠⒭ ⒯⒣⒠ ⒧⒜⒵⒴ ⒟⒪⒢',
        ],
        // Unwanted Interpolation
        ['$HOME $ENV{\'HOME\'} %d %s {0} %*.*s'],
        // PHP code
        ['echo \'hello world\'; exit(); for($i=32;$i<120;++$i){echo \chr($i);}'],
    ];
}
