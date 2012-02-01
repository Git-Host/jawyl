(function($){
	$.fn.extend({
		accordion2:function(){
			this.extend(this.___acc___);
			return this;		
		},
		___acc___:{
			construct:function(options){
							var src;
							for (var k = 0, max1 = this.length; k < max1; k++)
								if (!$(this.get(k)).hasClass("ui-accordion"))
								{
									src = this.get(k);
									src.style.display = "none";
									var theId = src.id;
									src.id = theId+"_old";
									var acc = jQuery('<div id="'+theId+'" class="ui-accordion"></div>');
									$(src).before(acc);
									//acc.appendTo($(src).parent().get(0));
									var roots = $(src).children();
									var h;
									var c;
									for (var i = 0, max = roots.length; i < max; i++){
										$('<div id="'+theId+'_accpart_'+(i+1)+'"><div id="'+theId+'_accheader_'+(i+1)+'" class="ui-accordion-header ui-accordion-collapsed"></div><div id="'+theId+'_acccontainer_'+(i+1)+'" class="ui-accordion-container" style="display:none;"></div></div>').appendTo(acc.get(0));
										h = $($(roots.get(i)).children().get(0));
										c = $($(roots.get(i)).children().get(1));
										h.appendTo("#"+theId+'_accheader_'+(i+1));
										c.appendTo("#"+theId+'_acccontainer_'+(i+1));
									}
									$(src).remove();
									$("div.ui-accordion-header",acc.get(0)).live("click",function(e){
										params = this.id.split("_");
										var w = $(this).closest("div.ui-accordion");
										w.accordion2().expand(Number(params[2])-1);
									});	
									acc.accordion2().init();
								}
							return $(this.origin);
						},
			init:function(){
					var index = $.cookie(this.get(0).id+"_current");
					if (index)
						this.accordion2().expand(Number(index),0);
					},
			expand:function(index,speed){
					var h = $("#"+this.get(0).id+"_accheader_"+(index+1),this);
					var c = $("#"+this.get(0).id+"_acccontainer_"+(index+1),this);
					var displ = c.css("display");
					if (displ == "none"){
						var conts = $("div.ui-accordion-container",this);
						for (var i=0, max = conts.length; i < max; i++)
							if (conts.get(i).style.display != "none"){
								h = h.add($("#"+conts.get(i).id.replace("acccontainer","accheader")).get(0));
								c = c.add(conts.get(i));
							}	 
						if ((speed === null) || (speed === undefined)) speed = "slow";
						c.slideToggle(speed);
						h.toggleClass("ui-accordion-expanded").toggleClass("ui-accordion-collapsed");
						$.cookie(this.get(0).id+"_current",index);
						}
					}
			}
	});
})(jQuery);