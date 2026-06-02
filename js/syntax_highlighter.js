(function($) {

  /**
   * Add syntax highlighter for textarea.
   */
  Backdrop.behaviors.cssInjectorAceEditor = {
    attach: function(context, settings) {
      $('body').addClass('has-js');
      var editor = ace.edit("editor");
      editor.getSession().setUseWorker(false);
      editor.setTheme("ace/theme/chrome");
      editor.getSession().setMode("ace/mode/css");
      editor.setOptions({
        enableBasicAutocompletion: true,
        enableSnippets: true,
        enableLiveAutocompletion: settings.cssInjector.enableLiveAutocompletion
      });
      editor.commands.addCommand({
        name: "Toggle Fullscreen",
        bindKey: "F12",
        exec: function(editor) {
            editor.container.requestFullscreen();
        }
      });
      editor.getSession().on('change', function(e) {
        setTextareaValue();
      });

      var setTextareaValue = function() {
        $('#edit-css-text').val(editor.getValue());
      }

      $('.disable-ace').click(function() {
        var $this = $(this);
        $this.toggleClass('ace-disabled');
        $text = $this.text() == 'Disable syntax highlighter' ? 'Enable syntax highlighter' : 'Disable syntax highlighter';
        $this.text($text);
        $('.form-item-css-text .form-textarea-wrapper, .ace-editor').toggle();
        $('.css-injector-edit .ace-editor-fullscreen.help').toggle();
      });

    }
  }

})(jQuery)
