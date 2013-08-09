(function($){
	$.fn.extend({
		fileupload:function(){
			this.extend(this.__upl__);
			this.extend(this.__fupl__);
			return this;
		},
		imageupload:function(){
			this.extend(this.__upl__);
			this.extend(this.__iupl__);
			return this;
		},
		__upl__:{
			__btn__:{},
			__uploader:function(btn){
						var offset = btn.offset();
						var fu = $('<input id="'+this.get(0).id+'_fu" class="upl-hidden-input" style="position:absolute;z-index:10000; -moz-opacity:0; filter:alpha(opacity: 0);opacity: 0;left:'+offset.left+';top:'+offset.top+'" type="file" name="'+this.data("name")+'" />');
						fu = $(fu.get(0));
						fu.__upl__.__btn__ = btn.get(0);
						/*alert(this.data("progressurl"));
						if (this.data("progressurl")){
						}
						fu.__*/
						fu.appendTo(document.body);
						
						offset = fu.offset();
						fu.css("left",offset.left - (fu.outerWidth() - btn.outerWidth()));
						fu.mouseleave(function(e){
								$(this).remove();
						}).mousemove(function(e){
							var uplbtn = $($(this).__upl__.__btn__);
							var offset = uplbtn.offset();
							if ((e.pageX < offset.left) 
								|| (e.pageX > (offset.left+uplbtn.outerWidth()))
								|| (e.pageY < offset.top)
								|| (e.pageY > (offset.top+uplbtn.outerHeight()))
							) $(this).remove();
							}).get(0).onchange = function(e){
									var uplbtn = $($(this).__upl__.__btn__);
									var frm = uplbtn.closest("form");
									var action = document.URL;
									if (frm.length > 0) 
										action = frm.get(0).action?frm.get(0).action:action;
									var uploader = $('<iframe id="'+this.id+'_if" name="'+this.id+'_uploader" style="display:none;"></iframe><form id="'+this.id+'_frm" target="'+this.id+'_uploader" action="'+action+'" method="post" enctype="multipart/form-data"></form>');
									uploader.appendTo(document.body);
									$('input',frm.get(0)).clone().appendTo($('#'+this.id+'_frm').get(0));
									$(this).appendTo($('#'+this.id+'_frm').get(0));
									$('#'+this.id+'_if').load(function(e){
										if (this.src == "about:blank"){
											setTimeout( function() {uploader.remove();}, 0);
											return;
										}
										
										var doc = uploader.contents().get(0);
										if (doc.readyState && doc.readyState != 'complete') return;
										
										fuevent = new $.Event("onupload");
										fuevent.response = $(doc.body).html();
										fuevent.sender = $('#'+this.id.replace("_fu_if",""));
										$('#'+this.id.replace('_if','_frm')).remove();
										fuevent.sender.trigger(fuevent);
										this.src = "about:blank";
									});
									$('#'+this.id+'_frm').submit();
								};
				},
				__delete:function(){
							var fuevent = new $.Event("ondelete");
							fuevent.sender = this;
							if (!fuevent.sender.triggerHandler(fuevent)){
								var frm = $(this).closest("form");
								var action = document.URL;
								if (frm.length > 0) 
									action = frm.get(0).action?frm.get(0).action:action;
								var uploader = $('<form id="'+this.id+'_frm" action="'+action+'" method="post" style="display:none;" ></form>');
								uploader.appendTo(document.body);
								$('input',frm.get(0)).clone().appendTo(uploader.get(0));
								$('<input type="hidden" name="'+fuevent.sender.data("name")+'" value="" />').appendTo(uploader.get(0));
								uploader.ajaxForm({target:null,dataType:fuevent.sender.data("responsedatatype"),success:fuevent.sender.data("ondeleted")}).submit();
							}											
				},
				__setoptions:function(options){
					if (options){
						if (options.ProgressUrl)
							this.data("progressurl",options.ProgressUrl);
						if (options.Name)
							this.data("name",options.Name);
						if (options.ResponseDataType)
							this.data("responsedatatype",options.ResponseDataType);
						if (options.OnUpload)
							this.bind("onupload",options.OnUpload);
						if (options.OnBeforeDelete)
							this.bind("ondelete",options.OnBeforeDelete);
						if (options.OnDeleted)
							this.data("ondeleted",options.OnDeleted);
					}
				}
		},
		__iupl__:{
				setValue:function(value){
					var lnk = $('#'+this.get(0).id+'_link');
					if ((value) && (value.length > 1)){
						if (lnk.length == 0){
							var tbl = this.get(0).firstChild;
							var td = $("td",tbl).get(0);
							$(td).html('<a id="'+this.get(0).id+'_link" class="upl-img-thumb-link" href="'+value[1]+'"><image id="'+this.get(0).id+'_img" alt="'+value[0]+'" src="'+value[1]+'" class="upl-img-thumbnail" /></a>');
						}
						else {
							var img = $('#'+this.get(0).id+'_img');
							lnk.attr("href",value[1]);
							img.attr("src",value[1]);
							img.attr("alt",value[0]);
						}						
					} else if (lnk.length > 0) lnk.remove();						
				},
				construct:function(options){
					var src;
					var chld;
					
					var filename;
					var filelink;
					
					for (var i = 0, max = this.length; i < max; i++)
					if (!$(this.get(i)).hasClass("ui-img-uploader"))
					{
						src = this.get(i);
						$(src).addClass("ui-img-uploader");
						
						chld = $(src).children();
						
						filename = "";
						filelink = "";
						
						if (chld.length > 1){
							filename = $(chld.get(0)).text();
							filelink = $(chld.get(1)).text();
						} 
						
						if (filename != "")
							$(src).html('<table class="upl-container"><tr><td><a id="'+src.id+'_link" class="upl-img-thumb-link" target="_blank" href="'+filelink+'"><image id="'+src.id+'_img" alt="'+filename+'" src="'+filelink+'" class="upl-img-thumbnail" /></a></td><td><div><button id="'+src.id+'_uplbtn" class="upl-file-btn">Upload</button></div><div><button id="'+src.id+'_delbtn" class="del-file-btn">Delete</button></div></td></tr></table>');
						else
							$(src).html('<table class="upl-container"><tr><td></td><td><div><button id="'+src.id+'_uplbtn" class="upl-file-btn">Upload</button><button id="'+src.id+'_delbtn" class="del-file-btn">Delete</button></td></tr></table>');
						
						$('#'+src.id+'_uplbtn').click(function(e){return false;}).mouseenter(function(e){
							var w = $($(this).closest("div.ui-img-uploader").get(0));
							w.imageupload().__uploader($(this));
						});
						
						$('#'+src.id+'_delbtn').click(function(e){
							var w = $($(this).closest("div.ui-img-uploader").get(0));
							w.imageupload().__delete();
							return false; 
						});
						if (!options.name) options.name = src.name;
						$(src).imageupload().__setoptions(options);
				}
				
				return this;		
				}
		},		
		__fupl__:{
				setValue:function(value){
					var lnk = $('#'+this.get(0).id+'_link');
					if ((value) && (value.length > 1)){
						if (lnk.length == 0){
							var tbl = this.get(0).firstChild;
							var td = $("td",tbl).get(0);
							$(td).html('<a id="'+this.get(0).id+'_link" target="_blank" href="'+value[1]+'" class="upl-file-link">'+value[0]+'</a>');
						}
						else {
							lnk.attr("href",value[1]);
							lnk.html(value[0]);
						}						
					}
					else if (lnk.length > 0) lnk.remove();						
				},
				construct:function(options){
					var src;
					var chld;
					
					var filename;
					var filelink;
					
					for (var i = 0, max = this.length; i < max; i++)
					if (!$(this.get(i)).hasClass("ui-file-uploader"))
					{
						src = this.get(i);
						$(src).addClass("ui-file-uploader");
						chld = $(src).children();
						
						filename = "";
						filelink = "";
						
						if (chld.length > 1){
							filename = $(chld.get(0)).text();
							filelink = $(chld.get(1)).text();
						} 
						
						if (filename != "")
							$(src).html('<table class="upl-container"><tr><td><a id="'+src.id+'_link" href="'+filelink+'" class="upl-file-link">'+filename+'</a></td><td><button id="'+src.id+'_uplbtn" class="upl-file-btn">Upload</button><button id="'+src.id+'_delbtn" class="del-file-btn">Delete</button></td></tr></table>');
						else
							$(src).html('<table class="upl-container"><tr><td></td><td><button id="'+src.id+'_uplbtn" class="upl-file-btn">Upload</button><button id="'+src.id+'_delbtn" class="del-file-btn">Delete</button></td></tr></table>');
						
						$('#'+src.id+'_uplbtn').click(function(e){return false;}).mouseenter(function(e){
							var w = $($(this).closest("div.ui-file-uploader").get(0));
							w.fileupload().__uploader($(this));
						});
						
						$('#'+src.id+'_delbtn').click(function(e){
							var w = $($(this).closest("div.ui-file-uploader").get(0));
							w.fileupload().__delete();
							return false; 
						});
						if (!options.name) options.name = src.name;
						$(src).fileupload().__setoptions(options);
				}
				return this;		
				}
		}
	});
})(jQuery);