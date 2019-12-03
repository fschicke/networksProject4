void logic_check(int side){
    //Right side/server side is checking logic
    if(side == 1){
        padLY = padLY_c;
        if(ballX < (43/2)){
            if(scoreR < scoreR_c){
                //need to reset here as well...
                reset();
                //This means that you have scored!!!!
                scoreR = scoreR_c;
                return;
            }
            //TODO: need logic for if the score has been reset to 0...
            ballX = ballX_c;
            ballY = ballY_c;
            dx = dx_c;
            dy = dy_c;
        }

    //Left side/client side is checking logic
    } else if(side == 0){
        padRY = padRY_c;
        if(ballX > (43/2)){
            if(scoreL < scoreL_c){
                //need to reset here as well...
                reset();
                //This means that you have scored!!!!
                scoreL = scoreL_c;
                return;
            }
            //TODO: need logic for if the score has been reset to 0...
            ballX = ballX_c;
            ballY = ballY_c;
            dx = dx_c;
            dy = dy_c;
        }
    }
}
