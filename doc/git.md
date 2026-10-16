

## do this for small changes
```
git switch debug
git add .
git commit -m "..."
git push
```

## do this for release changes
```
git switch main
git merge --no-ff debug -m "..." #"release: v0.1.0"
git tag -a v0.1.0 -m "..." #"NukeCore v0.1.0"
git push origin main --tags

git switch debug
git merge main
git push
```

## fix wrong commit 
```
# undo commit, keep changes staged
git reset --soft HEAD~1

# fix the files
git add .
git commit -m "Correct commit"
git push --force-with-lease origin <branch>
```

