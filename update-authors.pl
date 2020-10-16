#!/usr/bin/perl
=pod

    update-authors.pl is part of Grapa.

    Grapa is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Grapa is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grapa.  If not, see <http://www.gnu.org/licenses/>.

=cut
use strict;
use warnings;

sub ReplaceAuthors {
  my @authors = @_;
  $_ eq 'bl0ckeduser <bl0ckedusersoft%gmail.com>' and $_ = 'Gabriel Cormier-Affleck <bl0ckedusersoft%gmail.com>' for @authors;
  $_ eq 'hekel <hekel%archlinux.info>' and $_ = 'Adam Erdman <hekel%archlinux.info>' for @authors;
  $_ eq 'infirit <infirit%gmail.com>' and $_ = 'Sander Sweers <infirit%gmail.com>' for @authors;
  $_ eq 'leigh123linux <leigh123linux%googlemail.com>' and $_ = 'Leigh Scott <leigh123linux%googlemail.com>' for @authors;
  $_ eq 'lyokha <alexey.radkov%gmail.com>' and $_ = 'Alexey Radkov <alexey.radkov%gmail.com>' for @authors;
  $_ eq 'Martin Wimpress <martin%cafe-desktop.org>' and $_ = 'Martin Wimpress <martin.wimpress%canonical.com>' for @authors;
  $_ eq 'Martin Wimpress <code%flexion.org>' and $_ = 'Martin Wimpress <martin.wimpress%canonical.com>' for @authors;
  $_ eq 'monsta <monsta%inbox.ru>' and $_ = 'Vlad Orlov <monsta%inbox.ru>' for @authors;
  $_ eq 'Monsta <monsta%inbox.ru>' and $_ = 'Vlad Orlov <monsta%inbox.ru>' for @authors;
  $_ eq 'Oz <nahumoz%gmail.com>' and $_ = 'Oz N Tiram <nahumoz%gmail.com>' for @authors;
  $_ eq 'Paolo Bacchilega <paobac%src.gnome.org>' and $_ = 'Paolo Bacchilega <paobac%gnome.org>' for @authors;
  $_ eq 'Paolo Bacchilega <paolo.bacchilega%libero.it>' and $_ = 'Paolo Bacchilega <paobac%gnome.org>' for @authors;
  $_ eq 'raveit <chat-to-me%raveit.de>' and $_ = 'Wolfgang Ulbrich <cafe%raveit.de>' for @authors;
  $_ eq 'raveit65 <chat-to-me%raveit.de>' and $_ = 'Wolfgang Ulbrich <cafe%raveit.de>' for @authors;
  $_ eq 'raveit65 <cafe%raveit.de>' and $_ = 'Wolfgang Ulbrich <cafe%raveit.de>' for @authors;
  $_ eq 'rbuj <robert.buj%gmail.com>' and $_ = 'Robert Buj <robert.buj%gmail.com>' for @authors;
  $_ eq 'Scott Balneaves <sbalneav%ltsp.org>' and $_ = 'Scott Balneaves <sbalneav%cafe-desktop.org>' for @authors;
  $_ eq 'sc0w <scow%riseup.net>' and $_ = 'ZenWalker <scow%riseup.net>' for @authors;
  $_ eq 'Victor Kareh <vkareh%vkareh.net>' and $_ = 'Victor Kareh <vkareh%redhat.com>' for @authors;
  $_ eq 'Wolfgang Ulbrich <chat-to-me%raveit.de>' and $_ = 'Wolfgang Ulbrich <cafe%raveit.de>' for @authors;
  return @authors;
}

sub GetCurrentAuthors {
  my @authors;
  open(FILE,"src/ui/engrampa.about") or die "Can't open src/ui/engrampa.about";
  while (<FILE>) {
    if (/^Authors=*(.+)$/) {
      @authors=split(";",$1);
    }
  }
  close FILE;
  return ReplaceAuthors(@authors);
}

sub GetNewAuthors {
  my @authors = `git log --pretty="%an <%ae>" --since "2012-01-01" -- . "_.h" "_.c" | sort | uniq | sed 's/@/%/g' | sed '/^cafe-i18n.*/d'`;
  chomp @authors;
  return ReplaceAuthors(@authors);
}

my @A = GetCurrentAuthors;
my @B = GetNewAuthors;
for (@B) {
  s/<\d+\+(.+?)%users\.noreply\.github\.com>/<$1%users\.noreply\.github\.com>/;
  s/<(.+?)%users\.noreply\.github\.com>/https:\/\/github.com\/$1/;
}
my @merged = sort { $a cmp $b } keys %{{map {($_ => 1)} (@A, @B)}};
print join(';',@merged) . ';';
