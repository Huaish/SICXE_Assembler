YELLOW='\033[1;33m'
GREEN='\033[1;32m'
NC='\033[0m' # No Color

cd ..
echo "${YELLOW}Cleaning...${NC}"
make clean
echo "${YELLOW}Compiling...${NC}"
make
echo "${GREEN}Done.${NC}"
