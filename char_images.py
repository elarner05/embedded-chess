import pygame

pygame.init()

screen = pygame.display.set_mode((500, 500))

chars = ["01110,10001,10101,10001,01110,00110,00001".split(","),
         "10001,10010,10100,11000,10100,10010,10001".split(","),
         "00000,00000,01110,00001,01111,10001,01111".split(","),
         "1001,1010,1100,1100,1010,1001,1001".split(",")
         ]
current = 0

size = 50
clock = pygame.time.Clock()
while True:
    clock.tick(60)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()
        
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                current+=1
                if current>len(chars)-1: 
                    current = 0
    
    screen.fill((200,200,200))
    for r, row in enumerate(chars[current]):
        for c, col in enumerate(row):
            if col == "1":
                pygame.draw.rect(screen, (0,0,0), (c*size, r*size, size, size))
            else:
                pygame.draw.rect(screen, (255,255,255), (c*size, r*size, size, size))

    pygame.display.update()

