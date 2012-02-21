(function($){
	$.fn.extend({
		window:function(){
			this.extend(this.____window____);
			return this;		
		},
		____window____:{
			isopen:function(){
				if (this.length > 0)
					return $('#wnd_'+this.context.id).css("display") != "none";
				return false;
			},
			open:function(options){
				var bg = document.body;
				if (options.modal){
					bg = $('div.ui-modal-window-background');
					if (bg.length == 0) {
						bg = $('<div class="ui-modal-window-background" style="position:absolute;z-index:50;width:100%;height:100%;left:0px;top:0px;display:block;"></div>');
						bg.appendTo(document.body);
					}
					bg.css("display","block");
				}
				
				var l = document.body.offsetWidth/2;
				var h = document.body.offsetHeight/2;
				
				var base = this;
				
				var wnd = $('#wnd_'+this.get(0).id);
				if (wnd.length == 0){
					wnd = $('<div id="wnd_'+this.get(0).id+'" class="ui-wnd" style="position:absolute;z-index:60;overflow:hidden;"><table class="wnd-header" style="width:100%;"><tr><td>'+(options.title?options.title:'')+'</td><td><button class="wnd-btn-close">close</button></td></tr></table><div class="wnd-content" style="width:100%;overflow:hidden;"></div></div>');
					$("button.wnd-btn-close",wnd).click(function(e){base.close();});
					wnd.draggable({handle:'table.wnd-header'}).resizable({
						stop:function(e){
							$("div.wnd-content",wnd).css("width",wnd.clientWidth+"px");
							$("div.wnd-content",wnd).css("height",(wnd.clientHeight - $("table.wnd-header",wnd).attr("offsetHeight"))+"px");
						}
					});
				}
				
				//if (!$.contains(bg,wnd))
				wnd.appendTo(bg);
				
				if (options.width)
					$("div.wnd-content",wnd).css("width",options.width+"px");
				else
					$("div.wnd-content",wnd).css("width","auto");
					
					
				if (options.height)
					$("div.wnd-content",wnd).css("height",options.height+"px");
				else
					$("div.wnd-content",wnd).css("height","auto");
				
				$("div.wnd-content",wnd).html(this);
				
				//this.appendTo($("div.wnd-content",wnd));
				//this.css("display","block");
				
				l = Math.floor(wnd.attr("offsetLeft") + wnd.attr("offsetWidth")/2);
				h = Math.floor(wnd.attr("offsetTop") + wnd.attr("offsetHeight")/2);
					
				//cnt.appendTo($("div.wnd-content",wnd));
				wnd.css("width",wnd.attr("offsetWidth")+"px");
				wnd.css("height",wnd.attr("offsetHeight")+"px");
				
				l = Math.floor(l - wnd.attr("offsetWidth")/2);
				h = Math.floor(h - wnd.attr("offsetHeight")/2);
				
				wnd.css("left",l + "px");
				wnd.css("top",h + "px");
			
				wnd.css("display","block");
				wnd.css("visibility","visible");
				alert(wnd.get(0).id);
				wnd.show("fast");
				return wnd;											
			},
			close:function(){
				//$("#wnd_"+this.get(0).id).hide("fast",function(e){$("div.ui-modal-window-background").css("display","none");});
				$("#wnd_"+this.get(0).id).css("display","none");
				$("div.ui-modal-window-background").css("display","none");
			}
		}
	});
})(jQuery);