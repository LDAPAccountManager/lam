#!/usr/bin/perl
#
# Copyright (C) 2001 Ignat Ikryanov <iignat@newmail.ru>
#

use strict;
my @INtext;

open (text, "ru_RU.po") || die "$!";
open (text1,">ru_RU.KOI8-R.po") || die "$!";

@INtext=<text>;
close(text);

foreach(@INtext)
{
    tr/¸יצףךוםדרשחץתפûגאןנמכהז‎ÿקסלטעüב‏¨ÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏÐÎËÄÆÝ‗×ÑÌÈÒÜÁÞ/£ÊÃÕËÅÎÇÛÝÚÈ‗ÆÙ×ÁÐÒÏÌÄÖÜÑÞÓÍÉÔØÂÀ³ךדץכומחû‎תטÿזשקבנעןלהצüס‏ףםיפרגא/;
    s/CP1251/KOI8-R/;

# You can alternatively use the following lines to change the conversion direction.
#    tr/£ÊÃÕËÅÎÇÛÝÚÈ‗ÆÙ×ÁÐÒÏÌÄÖÜÑÞÓÍÉÔØÂÀ³ךדץכומחû‎תטÿזשקבנעןלהצüס‏ףםיפרגא/¸יצףךוםדרשחץתפûגאןנמכהז‎ÿקסלטעüב‏¨ÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏÐÎËÄÆÝ‗×ÑÌÈÒÜÁÞ/;

    print text1;
}
close (text1);
