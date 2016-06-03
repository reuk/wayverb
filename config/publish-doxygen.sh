REPO_PATH=git@github.com:reuk/wayverb.git
HTML_PATH=build/documentation/html
COMMIT_USER="Documentation Builder"
COMMIT_EMAIL="reuben.thomas@me.com"
CHANGESET=$(git rev-parse --verify HEAD)

rm -rf ${HTML_PATH}
mkdir -p ${HTML_PATH}
git clone -b gh-pages "${REPO_PATH}" --single-branch ${HTML_PATH}

cd ${HTML_PATH}
git rm -rf .
cd -

cd build
cmake ..
make doc
cd -

cd ${HTML_PATH}
git add .
git config user.name "${COMMIT_USER}"
git config user.email ${COMMIT_EMAIL}"
git commit -m "Automated docs for changeset ${CHANGESET}."
git push origin gh-pages
cd -
