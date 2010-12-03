/*	
 *	jQuery dropmenu 1.1.3
 *	www.frebsite.nl
 *	Copyright (c) 2010 Fred Heusschen
 *	Licensed under the MIT license.
 *	http://www.opensource.org/licenses/mit-license.php
 */
 
 
(function($) {
	$.fn.dropmenu = function(options) {
		var isIE 	= $.browser.ie,
			isIE6	= isIE && $.browser.version < 7,
			isIE7	= isIE && $.browser.version < 8;


		return this.each(function() {

			var opts  = $.extend({}, $.fn.dropmenu.defaults, options),
				$menu = $(this),
				$topl = $menu.find('> li'),
				menuX = $menu.offset().left;

			if (opts.maxWidth == 0) {
				opts.maxWidth = $('body').width() - menuX;
			}

			//	UL itself and all LI's
			$menu
				.css({
					display		: 'block',
					listStyle	: 'none'
				})
				.find("li")
				.css({
					display		: 'block',
					listStyle	: 'none',
					position	: 'relative',
					margin		: 0,
					padding		: 0
				});
			
			
			var css = {
				display		: 'block',
				outline		: 'none'
			};
			if (opts.nbsp) css['whiteSpace'] = 'nowrap';
			
			//	all A's and SPANs
			$menu
				.find('li > a, li > span')
				.css(css);


			//	top-level LI's and top level A's and SPANs
			$topl
				.css({
					float		: 'right'
				})
				.find('> a, > span')
				.addClass('toplevel')
				.css({
					float		: 'right'
				});		

			//	all sub-ULs
			$menu
				.find('ul')
				.css({
					display		: 'none',
					position	: 'absolute',
					margin		: 0,
					padding		: 0
				});
			
			//	first sub-UL and second, third, etc. sub-ULs
			$topl
				.find('> ul')
				.css({
					left		: 0,
					top			: $topl.outerHeight()
				})
				.find('li > a, li > span')
				.addClass('sublevel')
				.parent()
				.find('ul')
				.css({
					top			: 0
				}).data('subsub', true);
			
			$topl
				


			//	IE fixes
			if (isIE6) {
				$menu.find('ul').css({
					lineHeight	: 0
				});
			}
			if (isIE6 || isIE7) {
				$menu.find('ul a, ul span').css({
					zoom		: 1
				});
			}


			$menu.find('a').click(function() {
				$('ul', $menu).hide();
				$('a, span', $menu).removeClass('hover');
			});

			$menu.find('li').hover(
				
				//	showing submenu
				function() {
					var listit = this,
						subnav = $.fn.dropmenu.getSubnav(listit),
						subcss = { zIndex: $.fn.dropmenu.zIndex++ };

					$(listit).find('> a, > span').addClass('hover');

					if (!subnav) return;
					if ($(subnav).is(":animated")) return;

					if ($.data(subnav, 'subsub')) {
						var distance  = $(listit).outerWidth(),
							itemWidth = $(listit).offset().left + distance - menuX,
							position  = (opts.maxWidth < itemWidth) ? "right" : "left";
						
						subcss[position] = distance;						
					}
					$(subnav).css(subcss);
					$.data(subnav, 'stayOpen', true);
					
					switch (opts.effect) {
						case 'slide':
							$(subnav).slideDown(opts.speed);
							break;
						
						case 'fade':
							$(subnav).fadeIn(opts.speed);
							break;
							
						default:
							$(subnav).show();
							break;
					}

				//	hiding submenu
				}, function() {
					var listit = this,
						subnav = $.fn.dropmenu.getSubnav(listit);

					if (!subnav) {
						$(listit).find('> a, > span').removeClass('hover');
						return;
					}

					$.data(subnav, 'stayOpen', false);
					setTimeout(function() {
						if ($.data(subnav, 'stayOpen')) return;
						$(listit).find('> a, > span').removeClass('hover');

						$('ul', subnav).hide();
						switch (opts.effect) {
							case 'slide':
								$(subnav).slideUp(opts.speed);
								break;
							
							case 'fade':
								$(subnav).fadeOut(opts.speed);
								break;
								
							default:
								$(subnav).hide();
								break;
						}
						
					}, opts.timeout);
				}
			);
		});
	};
	
	$.fn.dropmenu.getSubnav = function(ele) {
		if (ele.nodeName.toLowerCase() == 'li') {
			var subnav = $('> ul', ele);
			return subnav.length ? subnav[0] : null;
		} else {
			return ele;
		}
	}
	
	$.fn.dropmenu.zIndex 	= 1000;
	$.fn.dropmenu.defaults 	= {
		effect			: 'none',		//	'slide', 'fade', or 'none'
		speed			: 'normal',		//	'normal', 'fast', 'slow', 100, 1000, etc
		timeout			: 250,
		nbsp			: false,
		maxWidth		: 0
	};
})(jQuery);