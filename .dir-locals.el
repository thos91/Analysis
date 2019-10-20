(defvar flycheck-include-path (list 
                               "/home/neo/Code/WAGASCI/Analysis/include"
                               "/home/neo/Code/ROOT/6-18-00/include"
                               "/home/neo/Code/WAGASCI/MIDAS/midas/include"
                               ))

(with-eval-after-load "flycheck"
  (setq flycheck-gcc-warnings `(,@flycheck-gcc-warnings	"no-write-strings"))
  (setq flycheck-gcc-include-path flycheck-include-path)
  (setq flycheck-clang-include-path flycheck-include-path)
  )

((nil . ((eval . (flycheck-mode 1))
         (cmake-ide-dir . "/home/neo/Code/WAGASCI/Analysis/")
         (cmake-ide-build-dir . "/home/neo/Code/WAGASCI/Analysis/build"))))
