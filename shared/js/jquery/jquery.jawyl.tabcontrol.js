(function($){
	$.fn.extend({
		tabcontrol:function(){
			this.extend(this.____tabc____);
			return this;		
		},
		____tabc____:{
			construct:function(){
			var src;
			for (var k = 0, max1 = this.length; k < max1; k++)
			if (!$(this.get(k)).hasClass("ui-tabcontrol"))
			{
				src = this.get(k);
				src.style.display = "none";
				var theId = src.id;
				this.id = theId+"_old";
				var acc = jQuery('<table id="'+theId+'" class="ui-tabcontrol"><tr height="30px;"><td id="'+theId+'_tabs"></td></tr><tr><td id="'+theId+'_pages"></td></tr></table>');
				$(src).before(acc);
				var roots = $(src).children();
				var h;
				var c;
				var dspl = "block";
				var tabstate = "active";
				for (var i = 0, max = roots.length; i < max; i++){
				    if (i > 0){
				    	dspl = "none";
				    	tabstate  = "inactive";
				    }
					$('<div id="'+theId+'_tab_'+(i+1)+'" class="ui-tabcontrol-tab ui-tab-'+tabstate+'"></div>').appendTo("#"+theId+'_tabs');
					$('<div id="'+theId+'_page_'+(i+1)+'" class="ui-tabcontrol-page" style="display:'+dspl+';height:100%;"></div>').appendTo("#"+theId+'_pages');
					h = $($(roots.get(i)).children().get(0));
					c = $($(roots.get(i)).children().get(1));
					h.appendTo("#"+theId+'_tab_'+(i+1));
					c.appendTo("#"+theId+'_page_'+(i+1));
				}
				$(src).remove();
				$("#"+theId+"_tabs").children("div.ui-tabcontrol-tab").bind("click",function(e){
					params = this.id.split("_");
					var w = $(this).closest("table.ui-tabcontrol");
					w.tabcontrol().showtab(Number(params[2])-1);
					});	
				acc.tabcontrol().inittc();
			}
		return $(this.origin);
	},
	inittc:function(){
		var index = $.cookie(this.get(0).id+"_current");
		if (index)
			this.tabcontrol().showtab(Number(index),0);
	},
	showtab:function(index,speed){
		var h = $("#"+this.get(0).id+"_tab_"+(index+1),this);
		var p = $("#"+this.get(0).id+"_page_"+(index+1),this);
		if (p.length > 0){
			var displ = p.get(0).style.display;
			if (displ == "none"){
				var conts = $('#'+this.get(0).id+'_pages').children("div.ui-tabcontrol-page");
				var tohide;
				for (var i=0, max = conts.length; i < max; i++)
					if (conts.get(i).style.display != "none"){
						h = h.add($("#"+conts.get(i).id.replace("page","tab")).get(0));
						if (tohide)
						tohide.add($(conts.get(i)))
						else
						tohide = $(conts.get(i));
					}
				h.toggleClass("ui-tab-active").toggleClass("ui-tab-inactive");
				if ((speed === null) || (speed === undefined)) speed = "normal";
				$.cookie(this.get(0).id+"_current",index);
				if (tohide)
				tohide.fadeOut(speed,function(){
					this.style.display = "none";
					p.fadeOut(0,function(){this.style.display = "block";p.fadeIn(speed);});
				});
				else
				p.fadeOut(0,function(){this.style.display = "block";p.fadeIn(speed);});
			}
		}
	}
	}});
})(jQuery);